#include "../include/VirtualServers.hpp"

VirtualServers::VirtualServers() { }

VirtualServers::VirtualServers(const VirtualServers &other) 
{
	*this = other;
}

VirtualServers &VirtualServers::operator=(const VirtualServers &rhs)
{
	if (this == &rhs)
		return (*this);
	this->_listen = rhs._listen;
	this->_server_name = rhs._server_name;
	this->_root = rhs._root;
	this->_index = rhs._index;
	this->_autoindex = rhs._autoindex;
	this->_locations = rhs._locations;
	this->_error_pages = rhs._error_pages;
	this->_client_max_body_size = rhs._client_max_body_size;
	this->_return = rhs._return;
	this->_server_address = rhs._server_address;
	this->_listen_fd = rhs._listen_fd;
	return (*this);
}

VirtualServers::~VirtualServers() {}

/* spliting line by separator */
std::vector<std::string> splitParametrs(std::string &input)
{
    std::vector<std::string> words;
    std::istringstream stream(input);
    std::string word;

    while (stream >> word) {
        words.push_back(word);
    }

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

/*************************************************************************************/
	std::cout << "******************Parámetros*********************" << std::endl;
	for (size_t i=0; i< parametrs.size(); i++){std::cout << parametrs[i] << std::endl;}
/**************************************************************************************/
	std::cout << "******************Parsing*********************" << std::endl;
	for (size_t i = 0; i < parametrs.size(); i++)
	{
		std::cout << "parametrs[" << i << "]: (" << parametrs[i] << ")" << std::endl;
		if (parametrs[i] == "listen" && (i + 1) < parametrs.size() && flag_loc)
		{
			//¿ Si hay varios listen ?
			server.setPort(parametrs[++i]);
			std::cout << "listen: " << server.getPort() << std::endl;
		}
		else if (parametrs[i] == "server_name" && (i + 1) < parametrs.size() && flag_loc)
		{
			if (!server.getServerName().empty())
				throw  ErrorException("Server_name is duplicated");
			server.setServerName(parametrs[++i]);
			std::cout << "server_name: " << server.getServerName() << std::endl;
		}
		else if (parametrs[i] == "root" && (i + 1) < parametrs.size() && flag_loc)
		{
			if (!server.getRoot().empty())
				throw  ErrorException("Root is duplicated");
			server.setRoot(parametrs[++i]);
			std::cout << "root: " << server.getRoot() << std::endl;
		}
		else if (parametrs[i] == "index" && (i + 1) < parametrs.size() && flag_loc)
		{
			if (!server.getIndex().empty())
				throw  ErrorException("Index is duplicated");
			server.setIndex(parametrs[++i]);
			std::cout << "index: " << server.getIndex() << std::endl;
		}
		else if (parametrs[i] == "autoindex" && (i + 1) < parametrs.size() && flag_loc)
		{
			if (flag_autoindex)
				throw ErrorException("Autoindex of server is duplicated");
			server.setAutoindex(parametrs[++i]);
			flag_autoindex = true;
			std::cout << "autoindex: " << server.getAutoindex() << std::endl;
		}
		else if (parametrs[i] == "location" && (i + 1) < parametrs.size())
		{
			std::string	path;
			i++;
			if (parametrs[i] == "{" || parametrs[i] == "}")
				throw  ErrorException("Wrong character in server scope{}");
			path = parametrs[i];
			std::vector<std::string> codes;
			if (parametrs[++i] != "{")
				throw  ErrorException("Wrong character in server scope{}");
			i++;
			std::cout << "location: " << std::endl;
			while (i < parametrs.size() && parametrs[i] != "}")
			{
				codes.push_back(parametrs[i++]);
				std::cout << "(" << parametrs[i-1] << ")";
			}
			std::cout << std::endl;
			server.setLocation(path, codes);
			if (i < parametrs.size() && parametrs[i] != "}")
				throw  ErrorException("Wrong character in location scope{}");
			flag_loc = 0;
		}	
		else if (parametrs[i] == "error_page" && (i + 1) < parametrs.size() && flag_loc)
		{
			while (++i < parametrs.size())
			{
				error_codes.push_back(parametrs[i]);
				if (parametrs[i].find(';') != std::string::npos)
					break ;
				if (i + 1 >= parametrs.size())
					throw ErrorException("Wrong character out of server scope{}");
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
		server.setRoot("/;");
	if (server.getIndex().empty())
		server.setIndex("index.html;");
	if (ConfigFile::checkPath(server.getRoot()) == -1  || ConfigFile::checkFile(server.getIndex(), 4) == -1)
		throw ErrorException("Index from config file not found or unreadable");
	if (!server.getPort())
		throw ErrorException("Port not found");
	server.setErrorPages(error_codes);
	if (!server._isValidErrorPages())
		throw ErrorException("Incorrect path for error page or number of error");
}

bool VirtualServers::_isValidErrorPages()
{
	std::map<short, std::string>::const_iterator it;
	for (it = this->_error_pages.begin(); it != this->_error_pages.end(); it++)
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
	_listen_fd = 0;
	_return = "";
	_error_pages[301] = "error_pages/301.html";
	_error_pages[302] = "error_pages/302.html";
	_error_pages[400] = "error_pages/400.html";
	_error_pages[401] = "error_pages/401.html";
	_error_pages[402] = "error_pages/402.html";
	_error_pages[403] = "error_pages/403.html";
	_error_pages[404] = "error_pages/404.html";
	_error_pages[405] = "error_pages/405.html";
	_error_pages[406] = "error_pages/406.html";
	_error_pages[500] = "error_pages/500.html";
	_error_pages[501] = "error_pages/501.html";
	_error_pages[502] = "error_pages/502.html";
	_error_pages[503] = "error_pages/503.html";
	_error_pages[505] = "error_pages/505.html";
	_error_pages[505] = "error_pages/505.html";
	_createServer(config, *this);
}

/*********************SETTERS***********************/
int VirtualServers::ft_stoi(std::string str)
{
    std::stringstream ss(str);
    if (str.length() > 10)
        throw std::exception();
    for (size_t i = 0; i < str.length(); ++i)
    {
        if(!isdigit(str[i]))
            throw std::exception();
    }
    int res;
    ss >> res;
    return (res);
}

void VirtualServers::setPort(std::string parametr)
{
	unsigned int port;
	
	port = 0;
	_checkToken(parametr);
	for (size_t i = 0; i < parametr.length(); i++)
	{
		if (!std::isdigit(parametr[i]))
			throw ErrorException("Wrong syntax: port");
	}
	port = ft_stoi((parametr));
	if (port < 1 || port > 65636)
		throw ErrorException("Wrong syntax: port");
	this->_listen = (uint16_t) port;
}

void VirtualServers::setServerName(std::string server_name)
{
	_checkToken(server_name);
	this->_server_name = server_name;
}

void VirtualServers::setRoot(std::string root)
{
	_checkToken(root);
	if (ConfigFile::getTypePath(root) == 2)
	{
		this->_root = root;
		return ;
	}
	char dir[1024];
	getcwd(dir, 1024);
	std::string full_root = dir + root;
	if (ConfigFile::getTypePath(full_root) != 2)
		throw ErrorException("Wrong syntax: root");
	this->_root = full_root;
}

void VirtualServers::setIndex(std::string index)
{
	_checkToken(index);
	this->_index = index;
}

void VirtualServers::setAutoindex(std::string autoindex)
{
	_checkToken(autoindex);
	if (autoindex != "on" && autoindex != "off")
		throw ErrorException("Wrong syntax: autoindex");
	if (autoindex == "on")
		this->_autoindex = true;
}

/* parsing and set locations */
void VirtualServers::setLocation(std::string path, std::vector<std::string> parametr)
{
	Location new_location;
	std::vector<std::string> methods;
	bool flag_methods = false;
	bool flag_autoindex = false;
	int valid;

	new_location.setPath(path);
	for (size_t i = 0; i < parametr.size(); i++)
	{
		if (parametr[i] == "root" && (i + 1) < parametr.size())
		{
			if (!new_location.getRootLocation().empty())
				throw ErrorException("Root of location is duplicated");
			_checkToken(parametr[++i]);
			if (ConfigFile::getTypePath(parametr[i]) == 2)
				new_location.setRootLocation(parametr[i]);
			else
				new_location.setRootLocation(this->_root + parametr[i]);
		}
		else if ((parametr[i] == "allow_methods" || parametr[i] == "methods") && (i + 1) < parametr.size())
		{
			if (flag_methods)
				throw ErrorException("Allow_methods of location is duplicated");
			std::vector<std::string> methods;
			while (++i < parametr.size())
			{
				if (parametr[i].find(";") != std::string::npos)
				{
					_checkToken(parametr[i]);
					methods.push_back(parametr[i]);
					break ;
				}
				else
				{
					methods.push_back(parametr[i]);
					if (i + 1 >= parametr.size())
						throw ErrorException("Token is invalid");
				}
			}
			new_location.setMethods(methods);
			flag_methods = true;
		}
		else if (parametr[i] == "autoindex" && (i + 1) < parametr.size())
		{
			if (path == "/cgi-bin")
				throw ErrorException("Parametr autoindex not allow for CGI");
			if (flag_autoindex)
				throw ErrorException("Autoindex of location is duplicated");
			_checkToken(parametr[++i]);
			new_location.setAutoindex(parametr[i]);
			flag_autoindex = true;
		}
		else if (parametr[i] == "index" && (i + 1) < parametr.size())
		{
			if (!new_location.getIndexLocation().empty())
				throw ErrorException("Index of location is duplicated");
			_checkToken(parametr[++i]);
			new_location.setIndexLocation(parametr[i]);
		}
		else if (parametr[i] == "return" && (i + 1) < parametr.size())
		{
			if (path == "/cgi-bin")
				throw ErrorException("Parametr return not allow for CGI");
			if (!new_location.getReturn().empty())
				throw ErrorException("Return of location is duplicated");
			_checkToken(parametr[++i]);
			new_location.setReturn(parametr[i]);
		}
		else if (parametr[i] == "alias" && (i + 1) < parametr.size())
		{
			if (path == "/cgi-bin")
				throw ErrorException("Parametr alias not allow for CGI");
			if (!new_location.getAlias().empty())
				throw ErrorException("Alias of location is duplicated");
			_checkToken(parametr[++i]);
			new_location.setAlias(parametr[i]);
		}
		else if (parametr[i] == "cgi_ext" && (i + 1) < parametr.size())
		{
			std::vector<std::string> extension;
			while (++i < parametr.size())
			{
				if (parametr[i].find(";") != std::string::npos)
				{
					_checkToken(parametr[i]);
					extension.push_back(parametr[i]);
					break ;
				}
				else
				{
					extension.push_back(parametr[i]);
					if (i + 1 >= parametr.size())
						throw ErrorException("Token is invalid");
				}
			}
			new_location.setCgiExtension(extension);
		}
		else if (parametr[i] == "cgi_path" && (i + 1) < parametr.size())
		{
			std::vector<std::string> path;
			while (++i < parametr.size())
			{
				if (parametr[i].find(";") != std::string::npos)
				{
					_checkToken(parametr[i]);
					path.push_back(parametr[i]);
					break ;
				}
				else
				{
					path.push_back(parametr[i]);
					if (i + 1 >= parametr.size())
						throw ErrorException("Token is invalid");
				}
				if (parametr[i].find("/python") == std::string::npos && parametr[i].find("/bash") == std::string::npos)
					throw ErrorException("cgi_path is invalid");
			}
			new_location.setCgiPath(path);
		}
		else if (i < parametr.size())
			throw ErrorException("Parametr in a location is invalid");
	}
	if (new_location.getPath() != "/cgi-bin" && new_location.getIndexLocation().empty())
		new_location.setIndexLocation(this->_index);
	valid = _isValidLocation(new_location);
	if (valid == 1)
		throw ErrorException("Failed CGI validation");
	else if (valid == 2)
		throw ErrorException("Failed path in locaition validation");
	else if (valid == 3)
		throw ErrorException("Failed redirection file in locaition validation");
	else if (valid == 4)
		throw ErrorException("Failed alias file in locaition validation");
	this->_locations.push_back(new_location);
}

/* check parametrs of location */
int VirtualServers::_isValidLocation(Location &location) const
{
	if (location.getPath() == "/cgi-bin")
	{
		if (location.getCgiPath().empty() || location.getCgiExtension().empty() || location.getIndexLocation().empty())
			return (1);


		if (ConfigFile::checkFile(location.getIndexLocation(), 4) < 0)
		{
			std::string path = location.getRootLocation() + location.getPath() + "/" + location.getIndexLocation();
			if (ConfigFile::getTypePath(path) != 1)
			{				
				std::string root = getcwd(NULL, 0);
				location.setRootLocation(root);
				path = root + location.getPath() + "/" + location.getIndexLocation();
			}
			if (path.empty() || ConfigFile::getTypePath(path) != 1 || ConfigFile::checkFile(path, 4) < 0)
				return (1);
		}
		if (location.getCgiPath().size() != location.getCgiExtension().size())
			return (1);
		std::vector<std::string>::const_iterator it;
		for (it = location.getCgiPath().begin(); it != location.getCgiPath().end(); ++it)
		{
			if (ConfigFile::getTypePath(*it) < 0)
				return (1);
		}
		std::vector<std::string>::const_iterator it_path;
		for (it = location.getCgiExtension().begin(); it != location.getCgiExtension().end(); ++it)
		{
			std::string tmp = *it;
			if (tmp != ".py" && tmp != ".sh" && tmp != "*.py" && tmp != "*.sh")
				return (1);
			for (it_path = location.getCgiPath().begin(); it_path != location.getCgiPath().end(); ++it_path)
			{
				std::string tmp_path = *it_path;
				if (tmp == ".py" || tmp == "*.py")
				{
					if (tmp_path.find("python") != std::string::npos)
						location._ext_path.insert(std::make_pair(".py", tmp_path));
				}
				else if (tmp == ".sh" || tmp == "*.sh")
				{
					if (tmp_path.find("bash") != std::string::npos)
						location._ext_path[".sh"] = tmp_path;
				}
			}
		}
		if (location.getCgiPath().size() != location.getExtensionPath().size())
			return (1);
	}
	else
	{
		if (location.getPath()[0] != '/')
			return (2);
		if (location.getRootLocation().empty()) {
			location.setRootLocation(this->_root);
		}
		if (ConfigFile::isFileExistAndReadable(location.getRootLocation() + location.getPath() + "/", location.getIndexLocation()))
			return (5);
		if (!location.getReturn().empty())
		{
			if (ConfigFile::isFileExistAndReadable(location.getRootLocation(), location.getReturn()))
				return (3);
		}
		if (!location.getAlias().empty())
		{
			if (ConfigFile::isFileExistAndReadable(location.getRootLocation(), location.getAlias()))
			 	return (4);
		}
	}
	return (0);
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

/* checks if there is such a default error code. If there is, it overwrites the path to the file,
otherwise it creates a new pair: error code - path to the file */
void VirtualServers::setErrorPages(std::vector<std::string> &parametr)
{
	if (parametr.empty())
		return;
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
		short code_error = ft_stoi(parametr[i]);
		if (statusCodeString(code_error)  == "Undefined" || code_error < 400)
			throw ErrorException ("Incorrect error code: " + parametr[i]);
		i++;
		std::string path = parametr[i];
		_checkToken(path);
		if (ConfigFile::checkFile(this->_root + path, 0) == -1 || ConfigFile::checkFile(this->_root + path, 4) == -1)
				throw ErrorException ("Error page file :" + this->_root + path + " is not accessible");
		std::map<short, std::string>::iterator it = this->_error_pages.find(code_error);
		if (it != _error_pages.end())
			this->_error_pages[code_error] = path;
		else
			this->_error_pages.insert(std::make_pair(code_error, path));
	}
}

void VirtualServers::setClientMaxBodySize(std::string parametr)
{
	unsigned long size;
	
	size = 0;
	_checkToken(parametr);
	for (size_t i = 0; i < parametr.length(); i++)
	{
		if (!std::isdigit(parametr[i]))
			throw ErrorException("Wrong syntax: client_max_body_size");
	}
	size = ft_stoi((parametr));
	if (size < 1 || size > 2147483647)
		throw ErrorException("Wrong syntax: client_max_body_size");
	this->_client_max_body_size = size;
}

void VirtualServers::setReturn(std::string parametr)
{
	_checkToken(parametr);
	this->_return = parametr;
}

void VirtualServers::setServerAddress(struct sockaddr_in server_address)
{
	this->_server_address = server_address;
}

void	VirtualServers::setFd(int fd)
{
	this->_listen_fd = fd;
}

/********************GETTERS***********************/
const uint16_t &VirtualServers::getPort() {return (_listen);}

const std::string &VirtualServers::getServerName() {return (_server_name);}

const std::string &VirtualServers::getRoot() {return (_root);}

const std::string &VirtualServers::getIndex() {return (_index);}

const bool &VirtualServers::getAutoindex() {return (_autoindex);}

const std::vector<Location> &VirtualServers::getLocations() {return (_locations);}

const std::map<short, std::string> &VirtualServers::getErrorPages() {return (_error_pages);}

const unsigned long &VirtualServers::getClientMaxBodySize() {return (_client_max_body_size);}

const std::string &VirtualServers::getReturn() {return (_return);}

const struct sockaddr_in &VirtualServers::getServerAddress() {return (_server_address);}

const int	&VirtualServers::getFd() {return (_listen_fd);}

/* the two functions below can be used later for response */
const std::string &VirtualServers::getPathErrorPage(short key)
{
	std::map<short, std::string>::iterator it = this->_error_pages.find(key);
	if (it == this->_error_pages.end())
		throw ErrorException("Error_page does not exist");
	return (it->second);
}

/* find location by a name */ //do not using in parser, created for server manager
const std::vector<Location>::iterator VirtualServers::getLocationKey(std::string key)
{
	std::vector<Location>::iterator it;
	for (it = this->_locations.begin(); it != this->_locations.end(); it++)
	{
		if (it->getPath() == key)
			return (it);
	}
	throw ErrorException("Error: path to location not found");
}

/* check is a properly end of parametr */
void VirtualServers::_checkToken(std::string &parametr)
{
	size_t pos = parametr.rfind(';');
	if (pos != parametr.size() - 1)
		throw ErrorException("Token is invalid");
	parametr.erase(pos);
}

/* check location for a dublicate */
bool VirtualServers::_checkLocations() const
{
	if (this->_locations.size() < 2)
		return (false);
	std::vector<Location>::const_iterator it1;
	std::vector<Location>::const_iterator it2;
	for (it1 = this->_locations.begin(); it1 != this->_locations.end() - 1; it1++) {
		for (it2 = it1 + 1; it2 != this->_locations.end(); it2++) {
			if (it1->getPath() == it2->getPath())
				return (true);
		}
	}
	return (false);
}
