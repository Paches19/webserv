/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServers.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 12:39:43 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/22 12:39:43 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualServers.hpp"

//*******************************************************************
// Constructores y destructor de la clase canónica
//*******************************************************************
VirtualServers::VirtualServers() { _port = 0; }
VirtualServers::VirtualServers(const VirtualServers &rhs)
{
	_port = rhs._port;
	_serverName = rhs._serverName;
	_root = rhs._root;
	_index = rhs._index;
	_autoindex = rhs._autoindex;
	_locations = rhs._locations;
	_errorPages = rhs._errorPages;
	_clientMaxBodySize = rhs._clientMaxBodySize;
	_ipAddress = rhs._ipAddress;
	_defaultServer = rhs._defaultServer;
}
VirtualServers &VirtualServers::operator=(const VirtualServers &rhs)
{
	if (this == &rhs)
		return (*this);
	_port = rhs._port;
	_serverName = rhs._serverName;
	_root = rhs._root;
	_index = rhs._index;
	_autoindex = rhs._autoindex;
	_locations = rhs._locations;
	_errorPages = rhs._errorPages;
	_clientMaxBodySize = rhs._clientMaxBodySize;
	_ipAddress = rhs._ipAddress;
	_defaultServer = rhs._defaultServer;
	return (*this);
}
VirtualServers::~VirtualServers() {}
VirtualServers::VirtualServers(std::string &config)
{
	_port = 0;
	_serverName = "";
	_root = "";
	_index = "";
	_autoindex = false;
	_clientMaxBodySize = MAX_CONTENT_LENGTH;
	_ipAddress.s_addr = 0;
	_defaultServer = false;
	// Hay que crear todas las páginas de errores. Estas son una muestra
	_errorPages[400] = "/error_pages/400.html";
	_errorPages[403] = "/error_pages/403.html";
	_errorPages[404] = "/error_pages/404.html";
	_errorPages[405] = "/error_pages/405.html";
	_errorPages[413] = "/error_pages/413.html";
	_errorPages[500] = "/error_pages/500.html";
	
	_createServer(config, *this);
	_checkServer(*this);
}

//*******************************************************************	
// Getters
//*******************************************************************
const uint16_t &VirtualServers::getPort() { return (_port); }

const std::string &VirtualServers::getServerName() { return (_serverName); }

const std::string &VirtualServers::getRoot() { return (_root); }

const std::string &VirtualServers::getIndex() { return (_index); }

const bool &VirtualServers::getAutoindex() { return (_autoindex); }

std::vector<Location> &VirtualServers::getLocations() { return (_locations); }

const std::string VirtualServers::getErrorPage(short i)
{
	std::map<short, std::string>::const_iterator it = _errorPages.find(i);
	if (it != _errorPages.end())
		return (it->second);
	return ("");
}

const unsigned long &VirtualServers::getClientMaxBodySize() { return (_clientMaxBodySize); }

const in_addr &VirtualServers::getIpAddress() { return (_ipAddress); }

const bool &VirtualServers::getDefaultServer() { return (_defaultServer); }

const std::map<short, std::string> &VirtualServers::getErrorPages() { return (_errorPages); }

//*******************************************************************
// Setters
//*******************************************************************
void VirtualServers::setPort(std::string parametr)
{
	std::string stringPort;
	std::string stringIP;

	if (parametr.find(':') != std::string::npos)
	{
		std::istringstream stream(parametr);
    	std::getline(stream, stringIP, ':');
    	std::getline(stream, stringPort);
	}
	else
	{
		stringPort = parametr;
		stringIP = "0.0.0.0";
	}
	setIpAddress(stringIP);

	if (!_defaultServer)
		Location::checkToken(stringPort);

	for (size_t i = 0; i < stringPort.length(); i++)
	{
		if (!std::isdigit(stringPort[i]))
			throw ErrorException("Wrong syntax: port");
	}
	unsigned int port = Location::ft_stoi((stringPort));
	if (port < 1 || port > 65636)
		throw ErrorException("Wrong syntax: port");
	_port = (uint16_t) port;
}

void VirtualServers::setIpAddress(std::string parametr)
{
	if (inet_aton(parametr.c_str(), &_ipAddress) == 0)
		throw ErrorException("Wrong syntax: ip address");
}

void VirtualServers::setServerName(std::string parametr)
{
	Location::checkToken(parametr);
	_serverName = parametr;
}

void VirtualServers::setRoot(std::string parametr)
{
	Location::checkToken(parametr);
	std::string auxRoot = ConfigFile::prefixPath(parametr);

	if (ConfigFile::checkPath(auxRoot) != 2)
		throw ErrorException("Wrong syntax: root");
	_root = auxRoot;
}

void VirtualServers::setIndex(std::string parametr)
{
	Location::checkToken(parametr);

	parametr = ConfigFile::adjustName(parametr);
	_index = parametr;
}

void VirtualServers::setAutoindex(std::string parametr)
{
	Location::checkToken(parametr);
	if (parametr != "on" && parametr != "off")
		throw ErrorException("Wrong syntax: autoindex");
	if (parametr == "on")
		_autoindex = true;
}

// Check if there is such a default error code. 
//   If there is, it overwrites the path to the file
//   Otherwise it creates a new pair: error code - path to the file
void VirtualServers::setErrorPage(short i, std::string parametr)
{
	if (i < 100 || i > 599)
		throw ErrorException("Wrong syntax: error_page");
	std::string path = ConfigFile::adjustName(parametr);
	_errorPages[i] = path;

}
void VirtualServers::setClientMaxBodySize(std::string parametr)
{
	unsigned long size;
	
	size = 0;
	Location::checkToken(parametr);
	for (size_t i = 0; i < parametr.length(); i++)
	{
		if (!std::isdigit(parametr[i]))
			throw ErrorException("Wrong syntax: client_max_body_size");
	}
	size = Location::ft_stoi((parametr));
	if (size < 1 || size > 2147483647)
		throw ErrorException("Wrong syntax: client_max_body_size");
	_clientMaxBodySize = size;
}

void VirtualServers::setLocation(std::vector<std::string> &parametrs, long unsigned &i)
{
	if (parametrs[i] == "{" || parametrs[i] == "}")
		throw  ErrorException("Wrong character in server scope{}");
	std::string path = parametrs[i];
	std::string modifier = "";
	if (parametrs[++i] != "{")
	{
		modifier = path;
		path = parametrs[i];
		if (parametrs[++i] != "{")
			throw  ErrorException("Wrong character in server scope{}");
	}
	path = ConfigFile::adjustName(path);
	
	i++;
	std::vector<std::string> codes;
	while (i < parametrs.size() && parametrs[i] != "}")
		codes.push_back(parametrs[i++]);
	if (i < parametrs.size() && parametrs[i] != "}")
		throw  ErrorException("Wrong character in location scope{}");
	Location  new_location(path, modifier, codes);
	std::map<short, std::string>::const_iterator it = _errorPages.begin();
	while (it != _errorPages.end())
	{
		if (new_location.getErrorPage(it->first).empty())
			new_location.setErrorPage(it->first, it->second);
		it++;
	}
	_locations.push_back(new_location);
}

//*******************************************************************
// Métodos de la clase
//*******************************************************************
std::vector<std::string> _splitParametrs(std::string &input)
{
    std::vector<std::string> words;
    std::istringstream stream(input);
    std::string word;

    while (stream >> word)
        words.push_back(word);
    return words;
}

void VirtualServers::_createServer(std::string &config, VirtualServers &server)
{
	std::vector<std::string>	parametrs;
	std::string	errorCodePath;
	short errorCode;
	bool	flag_autoindex = false;
	bool	flag_max_body_size = false;

	std::cout << "1. Creating server  ";
	parametrs = _splitParametrs(config);
	if (parametrs.size() < 3)
		throw  ErrorException("Failed server validation");
	size_t i = 0;
	if (parametrs[i] != "{" && parametrs[parametrs.size() - 1] != "}")
		throw  ErrorException("Wrong character in server scope{}");
	while (i < parametrs.size() - 2)
	{
		i++;
		if (parametrs[i] == "listen" && (i + 1) < parametrs.size() - 2)
		{
			++i;
			if (parametrs[i + 1] == "default_server;")
			{	
				if (!_defaultServer)
					_defaultServer = true;
				else
					throw  ErrorException("Default server already exists");
			}
			if (server.getPort() == 0)
				server.setPort(parametrs[i]);
			if (_defaultServer)
				++i;
		}
		else if (parametrs[i] == "server_name" && (i + 1) < parametrs.size() - 2)
		{
			++i;
			if (server.getServerName().empty())
				server.setServerName(parametrs[i]);
		}
		else if (parametrs[i] == "root" && (i + 1) < parametrs.size() - 2)
		{
			++i;
			if (server.getRoot().empty())
				server.setRoot(parametrs[i]);
		}
		else if (parametrs[i] == "index" && (i + 1) < parametrs.size() - 2)
		{
			++i;
			if (server.getIndex().empty())
				server.setIndex(parametrs[i]);
		}
		else if (parametrs[i] == "autoindex" && (i + 1) < parametrs.size() - 2)
		{
			++i;
			if (!flag_autoindex)
			{
				server.setAutoindex(parametrs[i]);
				flag_autoindex = true;
			}
		}
		else if (parametrs[i] == "location" && (i + 1) < parametrs.size() - 2)
		{
			setLocation(parametrs, ++i);
			if (parametrs[i] != "}")
				throw  ErrorException("Wrong character in server scope{}");
		}	
		else if (parametrs[i] == "error_page" && (i + 1) < parametrs.size() - 2)
		{
			errorCode = Location::ft_stoi(parametrs[++i]);
			Location::checkToken(parametrs[++i]);	
			errorCodePath = parametrs[i];
			setErrorPage(errorCode, errorCodePath);	
		}
		else if (parametrs[i] == "client_max_body_size" && (i + 1) < parametrs.size() - 2)
		{
			++i;
			if (!flag_max_body_size)
			{
				server.setClientMaxBodySize(parametrs[i]);
				flag_max_body_size = true;
			}
		}
		else
			throw  ErrorException("Unsupported directive: " + parametrs[i]);
	}
	std::cout << GREEN << " OM ! " << RESET << std::endl;
}

void VirtualServers::_checkServer(VirtualServers &server)
{
	std::cout << "2. Checking server" << std::endl;
	if (server.getPort() == 0)
		server.setPort("80;");
	if (server.getIpAddress().s_addr == 0)
		server.setIpAddress("0.0.0.0");
	if (server.getRoot().empty())
		server.setRoot("./;");
	if (server.getIndex().empty())
		server.setIndex("/index.html;");
	if (server.getServerName().empty())
		server.setServerName("localhost;");
		
	if (ConfigFile::checkPath(server.getRoot()) != IS_DIR)
		throw ErrorException("Root from config file not found or unreadable");
	std::string indexPath = server.getRoot() + server.getIndex();
	if (!ConfigFile::fileExistsAndReadable(indexPath))
		throw ErrorException("Index from config file not found or unreadable");

	std::cout << "        * Server name = " << server.getServerName() << std::endl;
	std::cout << "        * Listening = " << inet_ntoa(server.getIpAddress());
	std::cout << ": " << server.getPort() << std::endl;
	std::cout << "        * Root = " << server.getRoot() << std::endl;
	std::cout << "        * Index = " << server.getIndex() << std::endl;
	std::cout << GREEN << "        OK !" << RESET << std::endl;
	int error = 0;
	std::cout << "3. Checking locations" << std::endl;
	std::vector<Location> loc = server.getLocations();
	for (size_t i = 0; i < loc.size(); i++)
	{
		if (loc[i].getIndexLocation().empty() && loc[i].getReturn()[0].empty())
		{
			std::string defaultIndex = server.getIndex() + ";";
			loc[i].setIndexLocation(defaultIndex);
			if (loc[i].getAlias().empty())
			{
				std::string defaultRoot = server.getRoot() + ";";
				loc[i].setRootLocation(defaultRoot);
			}
		}
		
		error = loc[i].checkLocation(loc[i]);
		switch (error)
		{
			case 1:
				throw ErrorException("Failed CGI validation");
			case 2:
				throw ErrorException("Failed path in location validation");
			case 3:
				throw ErrorException("Failed redirection file in location validation");
			case 4:
				throw ErrorException("Failed alias file in location validation");
			default:
				std::cout << GREEN << "          OK !" << RESET << std::endl;
		}
	}
}
