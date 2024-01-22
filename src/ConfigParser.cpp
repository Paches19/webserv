#include "../include/ConfigParser.hpp"

ConfigParser::ConfigParser() { _nbServer = 0; }

ConfigParser::ConfigParser(const ConfigParser &other)
{
	_nbServer = other._nbServer;
	_serverConfig = other._serverConfig;
}

ConfigParser &ConfigParser::operator=(const ConfigParser &rhs)
{
	if (this == &rhs)
		return (*this);
	_nbServer = rhs._nbServer;
	_serverConfig = rhs._serverConfig;
	return (*this);
}

ConfigParser::~ConfigParser() { }

/************************ PRINTING servers configurations ************************/
int ConfigParser::print()
{
	for (size_t i = 0; i < _servers.size(); i++)
	{
		std::cout << YELLOW <<  "SERVER #" << i + 1 << RESET << std::endl;
		std::cout << CYAN << "Server name: " << RESET << _servers[i].getServerName() << std::endl;
		std::cout << CYAN << "Root: " << RESET << _servers[i].getRoot() << std::endl;
		std::cout << CYAN << "Index: " << RESET << _servers[i].getIndex() << std::endl;
		std::cout << CYAN <<  "Port: " << RESET << _servers[i].getPort() << std::endl;
		std::cout << CYAN << "Max BSize: " << RESET << _servers[i].getClientMaxBodySize() << std::endl;
		std::cout << CYAN << "Error pages: " << RESET << _servers[i].getErrorPages().size() << std::endl;
		std::map<short, std::string>::const_iterator it = _servers[i].getErrorPages().begin();
		while (it != _servers[i].getErrorPages().end())
		{
			std::cout << "   " << LIGHTRED << (*it).first << " - " << RESET << it->second << std::endl;
			++it;
		}
		std::cout << CYAN << "Locations: " << _servers[i].getLocations().size() << std::endl;
		std::vector<Location>::const_iterator itl = _servers[i].getLocations().begin();
		while (itl != _servers[i].getLocations().end())
		{
			std::cout << GREEN << "   name location: " << RESET << itl->getPath() << std::endl;
			std::cout << LIGHTRED << "      methods: " << RESET << itl->getPrintMethods() << std::endl;
			std::cout << LIGHTRED << "      index: " << RESET << itl->getIndexLocation() << std::endl;
			if (itl->getCgiPath().empty())
			{
				std::cout << LIGHTRED << "      root: " << RESET << itl->getRootLocation() << std::endl;
				if (!itl->getReturn().empty())
					std::cout << LIGHTRED << "      return: " << RESET << itl->getReturn() << std::endl;
				if (!itl->getAlias().empty())
					std::cout << LIGHTRED << "      alias: " << RESET << itl->getAlias() << std::endl;
			}
			else
			{
				std::cout << LIGHTRED << "      cgi root: " << RESET << itl->getRootLocation() << std::endl;
				std::cout << LIGHTRED << "      sgi_path: " << RESET << itl->getCgiPath().size() << std::endl;
				std::cout << LIGHTRED << "      sgi_ext: "  << RESET << itl->getCgiExtension().size() << std::endl;
			}
			++itl;
		}
		itl = _servers[i].getLocations().begin();
	}
	return (0);
}
/*********************************************************************************/

// Remove comments from char # to \n and empty lines
void removeCommentsAndEmptyLines(std::string &content)
{
    std::string result;
    std::istringstream iss(content);
    std::string line;

    while (std::getline(iss, line))
	{
        size_t firstCharPos = line.find_first_not_of(" \t");
        if (firstCharPos != std::string::npos)
		{
			int comment = line.find('#');
			if (comment != -1)
				line = line.substr(firstCharPos, comment - firstCharPos);
			else
				line = line.substr(firstCharPos);
			result += line;
			if (line != "")
				result += '\n';	
        }
    }
	if (!result.empty() && result[result.length() - 1] == '\n')
		result.erase(result.length() - 1);
    content = result;
}

// Finding the keyword "server" and returning the index of "{" (start of server)
size_t findStartServer (size_t start, std::string &content)
{
	size_t i;

	for (i = start; content[i]; i++)
	{
		if (content[i] == 's')
			break ;
		if (!isspace(content[i]))
			throw ConfigParser::ErrorException("Wrong character out of server scope{}");
	}
	if (!content[i])
		return (start);
	if (content.compare(i, 6, "server") != 0)
		throw ConfigParser::ErrorException("Wrong character out of server scope{}");
	i += 6;
	while (content[i] && isspace(content[i]))
		i++;
	if (content[i] == '{')
		return (i);
	else
		throw  ConfigParser::ErrorException("Wrong character out of server scope{}");
}

// Finding "}" (the end of a server) and returning its index
size_t findEndServer (size_t start, std::string &content)
{
	size_t	i;
	size_t	scope;
	
	scope = 0;
	for (i = start + 1; content[i]; i++)
	{
		if (content[i] == '{')
			scope++;
		if (content[i] == '}')
		{
			if (!scope)
				return (i);
			scope--;
		}
	}
	return (start);
}

// Spliting servers on separetly strings in vector
void ConfigParser::splitServers(std::string &content)
{
	size_t start = 0;
	size_t end = 1;

	removeCommentsAndEmptyLines(content);
	
	if (content.find("server", 0) == std::string::npos)
		throw ErrorException("Server did not find");
	while (start != end && start < content.length())
	{
		start = findStartServer(start, content);
		end = findEndServer(start, content);
		if (start == end)
			throw ErrorException("There is a problem with scope");
		_serverConfig.push_back(content.substr(start, end - start + 1));
		_nbServer++;
		start = end + 1;
	}
}

// The main function
int ConfigParser::initParser(const std::string &configFile)
{
	std::string		content;
	ConfigFile		file(configFile);
	
	// Checking and read config file
	if (file.checkPath(file.getPath()) == -1)
		throw ErrorException("File is invalid");
	if (file.checkFile(file.getPath(), 4) == -1)
		throw ErrorException("File is not accessible");
	content = file.readFile(configFile);
	if (content.empty())
		throw ErrorException("File is empty");
	//Splitting servers to strings
	splitServers(content);

//	for (size_t i = 0; i < this->_nbServer; i++)
//	{
//		std::cout << "---   SERVER #" << i + 1 << "   ---" << std::endl;
//		std::cout << _serverConfig[i] << std::endl;
//		std::cout << "--- END  SERVER #" << i + 1  << " ---\n" << std::endl;
//	}

	if (this->_serverConfig.size() != this->_nbServer)
		throw ErrorException("There is a problem with server configuration");
	// Creating vector of servers
	for (size_t i = 0; i < this->_nbServer; i++)
	{
		VirtualServers server(_serverConfig[i]);
		this->_servers.push_back(server);
	}
	return (0);
}
