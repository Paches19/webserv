#include "../include/VirtualServers.hpp"

VirtualServers::VirtualServers() { }

VirtualServers::VirtualServers(const VirtualServers &rhs)
{
	_listen = rhs._listen;
	_server_name = rhs._server_name;
	_root = rhs._root;
	_index = rhs._index;
	_autoindex = rhs._autoindex;
	_locations = rhs._locations;
	_error_pages = rhs._error_pages;
	_client_max_body_size = rhs._client_max_body_size;
	_return = rhs._return;
 }

VirtualServers &VirtualServers::operator=(const VirtualServers &rhs)
{
	if (this == &rhs)
		return (*this);
	_listen = rhs._listen;
	_server_name = rhs._server_name;
	_root = rhs._root;
	_index = rhs._index;
	_autoindex = rhs._autoindex;
	_locations = rhs._locations;
	_error_pages = rhs._error_pages;
	_client_max_body_size = rhs._client_max_body_size;
	_return = rhs._return;
	return (*this);
}

VirtualServers::~VirtualServers() {}

std::vector<std::string> splitParametrs(std::string &input)
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
	std::vector<std::string>	error_codes;
	int		flag_loc = 1;
	bool	flag_autoindex = false;

	parametrs = splitParametrs(config);
	if (parametrs.size() < 3)
		throw  ErrorException("Failed server validation");

	for (size_t i = 0; i < parametrs.size(); i++)
	{
		if (parametrs[i] == "listen" && (i + 1) < parametrs.size() && flag_loc)
		{
			if (server.getPort() != 0)
				throw  ErrorException("Listen is duplicated");
			server.setPort(parametrs[++i]);
		}
		else if (parametrs[i] == "server_name" && (i + 1) < parametrs.size() && flag_loc)
		{
			if (!server.getServerName().empty())
				throw  ErrorException("Server_name is duplicated");
			server.setServerName(parametrs[++i]);
		}
		else if (parametrs[i] == "root" && (i + 1) < parametrs.size() && flag_loc)
		{
			if (!server.getRoot().empty())
				throw  ErrorException("Root is duplicated");
			server.setRoot(parametrs[++i]);
		}
		else if (parametrs[i] == "index" && (i + 1) < parametrs.size() && flag_loc)
		{
			if (!server.getIndex().empty())
				throw  ErrorException("Index is duplicated");
			server.setIndex(parametrs[++i]);
		}
		else if (parametrs[i] == "autoindex" && (i + 1) < parametrs.size() && flag_loc)
		{
			if (flag_autoindex)
				throw ErrorException("Autoindex of server is duplicated");
			server.setAutoindex(parametrs[++i]);
			flag_autoindex = true;
		}
		else if (parametrs[i] == "location" && (i + 1) < parametrs.size())
		{
			i++;
			if (parametrs[i] == "{" || parametrs[i] == "}")
				throw  ErrorException("Wrong character in server scope{}");
			std::string path = parametrs[i];
			if (parametrs[++i] != "{")
				throw  ErrorException("Wrong character in server scope{}");
			i++;

			std::vector<std::string> codes;
			while (i < parametrs.size() && parametrs[i] != "}")
				codes.push_back(parametrs[i++]);
			if (i < parametrs.size() && parametrs[i] != "}")
				throw  ErrorException("Wrong character in location scope{}");
			Location  new_location(path, codes, _root);
			_locations.push_back(new_location);
			flag_loc = 0;
		}	
		else if (parametrs[i] == "error_page" && (i + 1) < parametrs.size() && flag_loc)
		{
			while (++i < parametrs.size())
			{
				if (i + 1 >= parametrs.size())
					throw ErrorException("Wrong character out of server scope{}");
				error_codes.push_back(parametrs[i]);
				if (parametrs[i].find(';') != std::string::npos)
					break ;
			}
		}
		else if (parametrs[i] == "client_max_body_size" && (i + 1) < parametrs.size() && flag_loc)
		{
			server.setClientMaxBodySize(parametrs[++i]);
		}
		else if (parametrs[i] == "return" && (i + 1) < parametrs.size() && flag_loc)
		{
			if (!server.getReturn().empty())
				throw  ErrorException("Return is duplicated");
			server.setReturn(parametrs[++i]);
		}
		else if (parametrs[i] != "}" && parametrs[i] != "{" )
		{
			if (!flag_loc)
				throw  ErrorException("Parametrs after location");
			else
				throw  ErrorException("Unsupported directive");
		}
	}
	if (server.getRoot().empty())
		server.setRoot("var/www;");
	if (server.getIndex().empty())
		server.setIndex("/index.html;");
	if (ConfigFile::checkPath(server.getRoot()) == -1)
		throw ErrorException("Root from config file not found or unreadable");
	if (ConfigFile::checkFile(server.getRoot() + server.getIndex(), 4) == -1)
		throw ErrorException("Index from config file not found or unreadable");
	if (!server.getPort())
		throw ErrorException("Port not found");
	server.setErrorPages(error_codes);
	if (!server._checkErrorPages())
		throw ErrorException("Incorrect path for error page or number of error");
}

bool VirtualServers::_checkErrorPages()
{
	std::map<short, std::string>::const_iterator it;
	for (it = _error_pages.begin(); it != _error_pages.end(); it++)
	{
		if (it->first < 100 || it->first > 599)
			return (false);
		if (ConfigFile::checkFile(getRoot() + it->second, 0) < 0 || ConfigFile::checkFile(getRoot() + it->second, 4) < 0)
			return (false);
	}
	return (true);
}

VirtualServers::VirtualServers(std::string &config)
{
	_listen = 0;
	_server_name = "";
	_root = "";
	_index = "";
	_autoindex = false;
	_client_max_body_size = MAX_CONTENT_LENGTH;
	_return = "";
	// Hay que crear todas las páginas de errores. Estas son una muestra
	_error_pages[403] = "/error_pages/403.html";
	_error_pages[404] = "/error_pages/404.html";
	_error_pages[405] = "/error_pages/405.html";
	
	_createServer(config, *this);
}


void VirtualServers::setPort(std::string parametr)
{
	unsigned int port;
	
	port = 0;
	Location::checkToken(parametr);
	for (size_t i = 0; i < parametr.length(); i++)
	{
		if (!std::isdigit(parametr[i]))
			throw ErrorException("Wrong syntax: port");
	}
	port = Location::ft_stoi((parametr));
	if (port < 1 || port > 65636)
		throw ErrorException("Wrong syntax: port");
	_listen = (uint16_t) port;
}

void VirtualServers::setServerName(std::string server_name)
{
	Location::checkToken(server_name);
	_server_name = server_name;
}

void VirtualServers::setRoot(std::string root)
{
	Location::checkToken(root);

	if (ConfigFile::getTypePath(root) == 2)
	{
		_root = root;
		return ;
	}
	char dir[1024];
	getcwd(dir, 1024);
	std::string full_root = dir + root;
	if (ConfigFile::getTypePath(full_root) != 2)
		throw ErrorException("Wrong syntax: root");
	_root = full_root;
}

void VirtualServers::setIndex(std::string index)
{
	Location::checkToken(index);
	if (index[0] != '/')
		index = "/" + index;
	_index = index;
}

void VirtualServers::setAutoindex(std::string autoindex)
{
	Location::checkToken(autoindex);
	if (autoindex != "on" && autoindex != "off")
		throw ErrorException("Wrong syntax: autoindex");
	if (autoindex == "on")
		_autoindex = true;
}

std::string statusCodeString(short statusCode)
{
    switch (statusCode)
    {
        case 100:
            return "Continue";
        case 101:
            return "Switching Protocol";
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 202:
            return "Accepted";
        case 203:
            return "Non-Authoritative Information";
        case 204:
            return "No Content";
        case 205:
            return "Reset Content";
        case 206:
            return "Partial Content";
        case 300:
            return "Multiple Choice";
        case 301:
            return "Moved Permanently";
        case 302:
            return "Moved Temporarily";
        case 303:
            return "See Other";
        case 304:
            return "Not Modified";
        case 307:
            return "Temporary Redirect";
        case 308:
            return "Permanent Redirect";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 406:
            return "Not Acceptable";
        case 407:
            return "Proxy Authentication Required";
        case 408:
            return "Request Timeout";
        case 409:
            return "Conflict";
        case 410:
            return "Gone";
        case 411:
            return "Length Required";
        case 412:
            return "Precondition Failed";
        case 413:
            return "Payload Too Large";
        case 414:
            return "URI Too Long";
        case 415:
            return "Unsupported Media Type";
        case 416:
            return "Requested Range Not Satisfiable";
        case 417:
            return "Expectation Failed";
        case 418:
            return "I'm a teapot";
        case 421:
            return "Misdirected Request";
        case 425:
            return "Too Early";
        case 426:
            return "Upgrade Required";
        case 428:
            return "Precondition Required";
        case 429:
            return "Too Many Requests";
        case 431:
            return "Request Header Fields Too Large";
        case 451:
            return "Unavailable for Legal Reasons";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 502:
            return "Bad Gateway";
        case 503:
            return "Service Unavailable";
        case 504:
            return "Gateway Timeout";
        case 505:
            return "HTTP Version Not Supported";
        case 506:
            return "Variant Also Negotiates";
        case 507:
            return "Insufficient Storage";
        case 510:
            return "Not Extended";
        case 511:
            return "Network Authentication Required";
        default:
            return "Undefined";
        }
}

// Check if there is such a default error code. 
//   If there is, it overwrites the path to the file
//   Otherwise it creates a new pair: error code - path to the file
void VirtualServers::setErrorPages(std::vector<std::string> &parametr)
{
	if (parametr.empty())
	{
		parametr.push_back("404");
		parametr.push_back("/error_pages/404.html;");
	}
	if (parametr.size() % 2 != 0)
		throw ErrorException ("Error page initialization faled");
	for (size_t i = 0; i < parametr.size() - 1; i++)
	{
		for (size_t j = 0; j < parametr[i].size(); j++) {
			if (!std::isdigit(parametr[i][j]))
				throw ErrorException("Error code is invalid");
		}
		if (parametr[i].size() != 3)
			throw ErrorException("Error code is invalid");
		short code_error = Location::ft_stoi(parametr[i]);
		if (statusCodeString(code_error)  == "Undefined" || code_error < 400)
			throw ErrorException ("Incorrect error code: " + parametr[i]);
		i++;
		std::string path = parametr[i];
		Location::checkToken(path);
		if (path[0] != '/')
			path = "/" + path;
		if (ConfigFile::checkFile(_root + path, 0) == -1 || ConfigFile::checkFile(_root + path, 4) == -1)
				throw ErrorException ("Error page file :" + _root + path + " is not accessible");
		std::map<short, std::string>::iterator it = _error_pages.find(code_error);
		if (it != _error_pages.end())
			_error_pages[code_error] = path;
		else
			_error_pages.insert(std::make_pair(code_error, path));
	}
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
	_client_max_body_size = size;
}

void VirtualServers::setReturn(std::string parametr)
{
	Location::checkToken(parametr);
	_return = parametr;
}

/********************GETTERS***********************/
const uint16_t &VirtualServers::getPort() { return (_listen); }

const std::string &VirtualServers::getServerName() { return (_server_name); }

const std::string &VirtualServers::getRoot() { return (_root); }

const std::string &VirtualServers::getIndex() { return (_index); }

const bool &VirtualServers::getAutoindex() { return (_autoindex); }

const std::vector<Location> &VirtualServers::getLocations() { return (_locations); }

const std::map<short, std::string> &VirtualServers::getErrorPages() { return (_error_pages); }

const unsigned long &VirtualServers::getClientMaxBodySize() { return (_client_max_body_size); }

const std::string &VirtualServers::getReturn() { return (_return); }
