#include "../include/ConfigParser.hpp"

ConfigParser::ConfigParser()
{
	this->_nb_server = 0;
}

ConfigParser::ConfigParser(const ConfigParser &other)
{
	this->_nb_server = other._nb_server;
	this->_server_config = other._server_config;
}

ConfigParser &ConfigParser::operator=(const ConfigParser &rhs)
{
	if (this == &rhs)
		return (*this);
	this->_nb_server = rhs._nb_server;
	this->_server_config = rhs._server_config;
	return (*this);
}

ConfigParser::~ConfigParser() { }

/* printing parametrs of servers from config file */
int ConfigParser::print()
{
	std::cout << "------------- Config -------------" << std::endl;
	for (size_t i = 0; i < _server_config.size(); i++)
	{
		std::cout << "Server #" << i + 1 << std::endl;
		std::cout << _server_config[i] << std::endl;
		std::cout << "-----------------------------" << std::endl;
	}

	std::cout << "------------- Config -------------" << std::endl;
	for (size_t i = 0; i < _servers.size(); i++)
	{
		std::cout << "Server #" << i + 1 << std::endl;
		std::cout << "Server name: " << _servers[i].getServerName() << std::endl;
		std::cout << "Root: " << _servers[i].getRoot() << std::endl;
		std::cout << "Index: " << _servers[i].getIndex() << std::endl;
		std::cout << "Port: " << _servers[i].getPort() << std::endl;
		std::cout << "Max BSize: " << _servers[i].getClientMaxBodySize() << std::endl;
		std::cout << "Error pages: " << _servers[i].getErrorPages().size() << std::endl;
		std::map<short, std::string>::const_iterator it = _servers[i].getErrorPages().begin();
		while (it != _servers[i].getErrorPages().end())
		{
			std::cout << (*it).first << " - " << it->second << std::endl;
			++it;
		}
		std::cout << "Locations: " << _servers[i].getLocations().size() << std::endl;
		std::vector<Location>::const_iterator itl = _servers[i].getLocations().begin();
		while (itl != _servers[i].getLocations().end())
		{
			std::cout << "name location: " << itl->getPath() << std::endl;
			std::cout << "methods: " << itl->getPrintMethods() << std::endl;
			std::cout << "index: " << itl->getIndexLocation() << std::endl;
			if (itl->getCgiPath().empty())
			{
				std::cout << "root: " << itl->getRootLocation() << std::endl;
				if (!itl->getReturn().empty())
					std::cout << "return: " << itl->getReturn() << std::endl;
				if (!itl->getAlias().empty())
					std::cout << "alias: " << itl->getAlias() << std::endl;
			}
			else
			{
				std::cout << "cgi root: " << itl->getRootLocation() << std::endl;
				std::cout << "sgi_path: " << itl->getCgiPath().size() << std::endl;
				std::cout << "sgi_ext: " << itl->getCgiExtension().size() << std::endl;
			}
			++itl;
		}
		itl = _servers[i].getLocations().begin();
		std::cout << "-----------------------------" << std::endl;
	}
	return (0);
}


/*remove comments from char # to \n */
void removeComments(std::string &content)
{
	size_t pos;

	pos = content.find('#');
	while (pos != std::string::npos)
	{
		size_t pos_end;
		pos_end = content.find('\n', pos);
		content.erase(pos, pos_end - pos);
		pos = content.find('#');
	}
}

/* deleting whitespaces in the start, end and in the content if more than one */
void removeWhiteSpace(std::string &content)
{
	size_t	i = 0;

	while (content[i] && isspace(content[i]))
		i++;
	content = content.substr(i);
	i = content.length() - 1;
	while (i > 0 && isspace(content[i]))
		i--;
	content = content.substr(0, i + 1);
}

/* finding a server begin and return the index of { start of server */
size_t findStartServer (size_t start, std::string &content)
{
	size_t i;

	for (i = start; content[i]; i++)
	{
		if (content[i] == 's')
			break ;
		if (!isspace(content[i]))
			throw  ConfigParser::ErrorException("Wrong character out of server scope{}");
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

/* finding a server end and return the index of } end of server */
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

/* spliting servers on separetly strings in vector */
void ConfigParser::splitServers(std::string &content)
{
	size_t start = 0;
	size_t end = 1;

	removeComments(content);
	removeWhiteSpace(content);
	if (content.find("server", 0) == std::string::npos)
		throw ErrorException("Server did not find");
	while (start != end && start < content.length())
	{
		start = findStartServer(start, content);
		end = findEndServer(start, content);
		if (start == end)
			throw ErrorException("There is a problem with scope");
		_server_config.push_back(content.substr(start, end - start + 1));
		_nb_server++;
		start = end + 1;
	}
}

/* checking and read config file, split servers to strings and creating vector of servers */
int ConfigParser::initParser(const std::string &config_file)
{
	std::string		content;
	ConfigFile		file(config_file);

	if (file.checkPath(file.getPath()) == -1)
		throw ErrorException("File is invalid");
	if (file.checkFile(file.getPath(), 4) == -1)
		throw ErrorException("File is not accessible");
	content = file.readFile(config_file);
	if (content.empty())
		throw ErrorException("File is empty");
	
	splitServers(content);

/************************PRINTING servers from config file************************/
	for (size_t i = 0; i < this->_nb_server; i++)
	{
		std::cout << "------------------------------ SERVER #" << i + 1 << " ------------------------" << std::endl;
		std::cout << _server_config[i] << std::endl;
	}
	std::cout << "---------------------------------- END SERVERS --------------------------------" << std::endl;
/*********************************************************************************/

	if (this->_server_config.size() != this->_nb_server)
		throw ErrorException("There is a problem with server configuration");

	for (size_t i = 0; i < this->_nb_server; i++)
	{
		VirtualServers server(_server_config[i]);
		this->_servers.push_back(server);
	}
	return (0);
}
