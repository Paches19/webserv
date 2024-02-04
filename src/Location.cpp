/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 12:39:09 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/22 12:39:09 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

//*******************************************************************
// Constructores y destructor de la clase canónica
//*******************************************************************
Location::Location() { }
Location::Location(const Location &other)
{
	_path = other._path;
	_root = other._root;
	_autoindex = other._autoindex;
	_index = other._index;
	_cgiPath = other._cgiPath;
	_cgiExt = other._cgiExt;
	_return = other._return;
	_alias = other._alias;
    _methods = other._methods;
	_extPath = other._extPath;
	_modifier = other._modifier;
	_clientMaxBodySize = other._clientMaxBodySize;
}
Location &Location::operator=(const Location &rhs)
{
	if (this != &rhs)
	{
		_path = rhs._path;
		_root = rhs._root;
		_autoindex = rhs._autoindex;
		_index = rhs._index;
		_cgiPath = rhs._cgiPath;
		_cgiExt = rhs._cgiExt;
		_return = rhs._return;
		_alias = rhs._alias;
		_methods = rhs._methods;
		_extPath = rhs._extPath;
		_modifier = rhs._modifier;
		_clientMaxBodySize = rhs._clientMaxBodySize;
    }
	return (*this);
}
Location::~Location() { }
Location::Location(std::string &path, std::string &modifier, std::vector<std::string> &parametr, std::string &r)
{
	_path = path;
	_root = r;
	_autoindex = false;
	_index = "";
	_return = "";
	_alias = "";
	_modifier = modifier;
	_clientMaxBodySize = MAX_CONTENT_LENGTH;
	_methods.reserve(5);
	_methods.push_back(1); // GET
	_methods.push_back(0); // POST
	_methods.push_back(0); // DELETE
	_methods.push_back(0); // PUT
	_methods.push_back(0); // HEAD

	// Configuración de los parámetros de la localización
	configureLocation(path, parametr);
}

//*******************************************************************
// Getters
//*******************************************************************
const std::string &Location::getModifier() const { return (_modifier); }

const std::string &Location::getPath() const { return (_path); }

const std::string &Location::getRootLocation() const { return (_root); }

const std::string &Location::getIndexLocation() const { return (_index); }

const std::vector<short> &Location::getMethods() const { return (_methods); }

const std::vector<std::string> &Location::getCgiPath() const { return (_cgiPath); }

const std::vector<std::string> &Location::getCgiExtension() const { return (_cgiExt); }

const bool &Location::getAutoindex() const { return (_autoindex); }

const std::string &Location::getReturn() const { return (_return); }

const std::string &Location::getAlias() const { return (_alias); }

const std::map<std::string, std::string> &Location::getExtensionPath() const { return (_extPath); }

const unsigned long &Location::getMaxBodySize() const { return (_clientMaxBodySize); }

const std::string Location::getErrorPage(short i) const
{
	std::map<short, std::string>::const_iterator it = _errorPages.find(i);
	if (it != _errorPages.end())
		return (it->second);
	return ("");
 }

 //*******************************************************************
 // Setters
 //*******************************************************************
void Location::setPath(std::string parametr) { _path = parametr; }

void Location::setRootLocation(std::string parametr)
{
	if (ConfigFile::getTypePath(parametr) != 2)
		throw ErrorException("root of location");
	_root = parametr;
}

void Location::setMethods(std::vector<std::string> methods)
{
	_methods[0] = 0;
	_methods[1] = 0;
	_methods[2] = 0;
	_methods[3] = 0;
	_methods[4] = 0;

	for (size_t i = 0; i < methods.size(); i++)
	{
		if (methods[i] == "GET")
			_methods[0] = 1;
		else if (methods[i] == "POST")
			_methods[1] = 1;
		else if (methods[i] == "DELETE")
			_methods[2] = 1;
		else if (methods[i] == "PUT")
			_methods[3] = 1;
		else if (methods[i] == "HEAD")
			_methods[4] = 1;
		else
			throw ErrorException("Allow method not supported " + methods[i]);
	}
}

void Location::setAutoindex(std::string parametr)
{
	if (parametr == "on" || parametr == "off")
		_autoindex = (parametr == "on");
	else
		throw ErrorException("Wrong autoindex");
}

void Location::setIndexLocation(std::string parametr)
{
	if (parametr[0] != '/')
		parametr = "/" + parametr;
	_index = parametr;
}

void Location::setReturn(std::string parametr) { _return = parametr; }

void Location::setAlias(std::string parametr) {	_alias = parametr; }

void Location::setCgiPath(std::vector<std::string> path) { _cgiPath = path; }

void Location::setCgiExtension(std::vector<std::string> extension) { _cgiExt = extension; }

void Location::setMaxBodySize(std::string parametr)
{
	for (size_t i = 0; i < parametr.length(); i++)
	{
		if (parametr[i] < '0' || parametr[i] > '9')
			throw ErrorException("Wrong syntax: client_max_body_size");
	}
	if (!ft_stoi(parametr))
		throw ErrorException("Wrong syntax: client_max_body_size");
	_clientMaxBodySize = ft_stoi(parametr);
}

void Location::setMaxBodySize(unsigned long parametr) { _clientMaxBodySize = parametr; }

void Location::setModifier(std::string parametr) { _modifier = parametr; }

void Location::setErrorPage(std::vector<std::string> &parametr)
{
	if (parametr.empty())
	{
		parametr.push_back("404");
		parametr.push_back("/error_pages/404.html");
	}
	for (size_t i = 0; i < parametr.size() - 1; i++)
	{
		short codeError = ft_stoi(parametr[i]);
		if (codeError < 100 || codeError > 599)
			throw ErrorException ("Incorrect error code: " + parametr[i]);
		i++;
		std::string path = parametr[i];
		if (path[0] != '/')
			path = "/" + path;
		std::map<short, std::string>::iterator it = _errorPages.find(codeError);
		if (it != _errorPages.end())
			_errorPages[codeError] = path;
		else
			_errorPages.insert(std::make_pair(codeError, path));
	}
}

//*******************************************************************
// Métodos de la clase
//*******************************************************************
void Location::configureLocation(std::string &path, std::vector<std::string> &parametr)
{
	std::vector<std::string>	methods;
	std::vector<std::string>	errorCodes;
	bool flag_methods = false;
	bool flag_autoindex = false;

	for (size_t i = 0; i < parametr.size(); i++)
	{
		if (parametr[i] == "error_page" && (i + 1) < parametr.size())
		{
			++i;
			if (ft_stoi(parametr[i]) < 100 || ft_stoi(parametr[i]) > 599)
					throw ErrorException("Wrong error code");
			errorCodes.push_back(parametr[i++]);
			checkToken(parametr[i]);	
			errorCodes.push_back(parametr[i]);	
		}
		else if (parametr[i] == "root" && (i + 1) < parametr.size())
		{
			checkToken(parametr[++i]);
			if (ConfigFile::getTypePath(parametr[i]) == 2)
				setRootLocation(parametr[i]);
			else
				setRootLocation(_root + parametr[i]);
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
					checkToken(parametr[i]);
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
			setMethods(methods);
			flag_methods = true;
		}
		else if (parametr[i] == "autoindex" && (i + 1) < parametr.size())
		{
			if (path == "/cgi-bin")
				throw ErrorException("Parametr autoindex not allow for CGI");
			if (flag_autoindex)
				throw ErrorException("Autoindex of location is duplicated");
			checkToken(parametr[++i]);
			setAutoindex(parametr[i]);
			flag_autoindex = true;
		}
		else if (parametr[i] == "index" && (i + 1) < parametr.size())
		{
			if (!getIndexLocation().empty())
				throw ErrorException("Index of location is duplicated");
			checkToken(parametr[++i]);
			setIndexLocation(parametr[i]);
		}
		else if (parametr[i] == "return" && (i + 1) < parametr.size())
		{
			if (path == "/cgi-bin")
				throw ErrorException("Parametr return not allow for CGI");
			if (!getReturn().empty())
				throw ErrorException("Return of location is duplicated");
			checkToken(parametr[++i]);
			setReturn(parametr[i]);
		}
		else if (parametr[i] == "alias" && (i + 1) < parametr.size())
		{
			if (path == "/cgi-bin")
				throw ErrorException("Parametr alias not allow for CGI");
			if (!getAlias().empty())
				throw ErrorException("Alias of location is duplicated");
			checkToken(parametr[++i]);
			setAlias(parametr[i]);
		}
		else if (parametr[i] == "cgi_ext" && (i + 1) < parametr.size())
		{
			std::vector<std::string> extension;
			while (++i < parametr.size())
			{
				if (parametr[i].find(";") != std::string::npos)
				{
					checkToken(parametr[i]);
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
			setCgiExtension(extension);
		}
		else if (parametr[i] == "cgi_path" && (i + 1) < parametr.size())
		{
			std::vector<std::string> path;
			while (++i < parametr.size())
			{
				if (parametr[i].find(";") != std::string::npos)
				{
					checkToken(parametr[i]);
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
			setCgiPath(path);
		}
		else if (i < parametr.size())
			throw ErrorException("Parametr in a location is invalid");
	}
	if (getPath() != "/cgi-bin" && getIndexLocation().empty())
		setIndexLocation(_index);
	setErrorPage(errorCodes);

	/*
	int valid = _checkLocation(*this);

	if (valid == 1)
		throw ErrorException("Failed CGI validation");
	else if (valid == 2)
		throw ErrorException("Failed path in location validation");
	else if (valid == 3)
		throw ErrorException("Failed redirection file in location validation");
	else if (valid == 4)
		throw ErrorException("Failed alias file in location validation");
	*/
}

void Location::checkToken(std::string &parametr)
{
	size_t pos = parametr.rfind(';');
	if (pos != parametr.size() - 1)
		throw ErrorException("Token is invalid");
	parametr.erase(pos);
}

int Location::ft_stoi(std::string str)
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

int Location::_checkLocation(Location &location) const
{
	if (location.getPath() == "/cgi-bin")
	{
		if (location.getCgiPath().empty() || location.getCgiExtension().empty() || location.getIndexLocation().empty())
			return (1);
		if (ConfigFile::checkFile(location.getIndexLocation(), 4) < 0)
		{
			std::string path = location.getRootLocation() + location.getPath() + location.getIndexLocation();
			if (ConfigFile::getTypePath(path) != 1)
			{				
				std::string root = getcwd(NULL, 0);
				location.setRootLocation(root);
				path = root + location.getPath() + location.getIndexLocation();
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
						location._extPath.insert(std::make_pair(".py", tmp_path));
				}
				else if (tmp == ".sh" || tmp == "*.sh")
				{
					if (tmp_path.find("bash") != std::string::npos)
						location._extPath[".sh"] = tmp_path;
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
			location.setRootLocation(_root);
		}
		if (ConfigFile::isFileExistAndReadable(location.getRootLocation() + location.getPath(), location.getIndexLocation()))
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
const Location* Location::selectLocation(const std::string& requestURI,
	const std::vector<Location>& locations)
{
	const Location* exactMatch = findExactMatch(requestURI, locations);
	if (exactMatch)
		return exactMatch;

	const Location* longestPrefixMatch = findLongestPrefixMatch(requestURI, locations);
	if (longestPrefixMatch && longestPrefixMatch->getModifier() == "^~")
		return longestPrefixMatch;

	return longestPrefixMatch;
}

const Location* Location::findExactMatch(const std::string& requestURI,
	const std::vector<Location>& locations)
{
	for (size_t i = 0; i < locations.size(); ++i)
	{
		if (locations[i].getModifier() == "=" && locations[i].getPath() == requestURI)
			return &locations[i];
	}
	return NULL;
}

const Location* Location::findLongestPrefixMatch(const std::string& requestURI,
	const std::vector<Location>& locations)
{
	const Location* longestMatch = NULL;
	size_t longestLength = 0;

	for (size_t i = 0; i < locations.size(); ++i)
	{
		if ((locations[i].getModifier() == "^~" || locations[i].getModifier().empty())
			&& startsWith(requestURI, locations[i].getPath()) &&
				locations[i].getPath().length() > longestLength)
		{
			longestMatch = &locations[i];
			longestLength = locations[i].getPath().length();
		}
	}
	return longestMatch;
}

bool Location::startsWith(const std::string& str, const std::string& prefix)
{
    return str.substr(0, prefix.size()) == prefix;
}

//*******************************************************************
// To print methods.  Remove before send the project
//*******************************************************************
std::string Location::getPrintMethods() const
{
	std::string res;
	if (_methods[4])
		res.insert(0, "HEAD");
	if (_methods[3])
	{
		if (!res.empty())
			res.insert(0, ", ");
		res.insert(0, "PUT");
	}
	if (_methods[2])
	{
		if (!res.empty())
			res.insert(0, ", ");
		res.insert(0, "DELETE");
	}
	if (_methods[1])
	{
		if (!res.empty())
			res.insert(0, ", ");
		res.insert(0, "POST");
	}
	if (_methods[0])
	{
		if (!res.empty())
			res.insert(0, ", ");
		res.insert(0, "GET");
	}
	return (res);
}
