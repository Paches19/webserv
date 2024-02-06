/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 12:38:47 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/06 12:56:25 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigFile.hpp"

//*******************************************************************
// Constructores y destructor de la clase canónica
//*******************************************************************
ConfigFile::ConfigFile() { }
ConfigFile::ConfigFile(const ConfigFile &other) { _path = other._path; }
ConfigFile &ConfigFile::operator=(const ConfigFile &rhs)
{
	if (this == &rhs)
		return (*this);
	_path = rhs._path;
	return (*this);
}
ConfigFile::ConfigFile(std::string const path) : _path(path) { }
ConfigFile::~ConfigFile() { }

//*******************************************************************
// Getters
//*******************************************************************
std::string ConfigFile::getPath() {	return (_path); }

int ConfigFile::getTypePath(std::string path)
{
	struct stat	buffer;

	if (path[0] != '/' && path[0] != '.')
		path = "./" + path;
	if (path[0] == '/')
		path = "." + path;
	if (stat(path.c_str(), &buffer) == 0)
	{
		if (buffer.st_mode & S_IFREG)
			return (1); //is file
		if (buffer.st_mode & S_IFDIR)
			return (2); //is folder
		return (3); //is something else
	}
	return (-1);
}

//*******************************************************************
// Métodos de la clase
//*******************************************************************

std::string ConfigFile::prefixPath(std::string const path)
{
	if (path[0] != '/' && path[0] != '.')
		return ("./" + path);
	if (path[0] == '/')
		return ("." + path);
	return (path);
}

int ConfigFile::checkPath(std::string path)
{
	struct stat	buffer;
	int			result;
	
	std::string expath = prefixPath(path);
	result = stat(expath.c_str(), &buffer);
	if (result == 0)
		return (1);
	return (-1);
}

// Check is the file exists and accessable
int	ConfigFile::checkFile(std::string  path, int mode)
{
	std::string expath = prefixPath(path);
	return (access(expath.c_str(), mode));
}

// Read from file to string
std::string	ConfigFile::readFile(std::string path)
{
	if (path.empty() || path.length() == 0)
		return (NULL);

	std::string expath = prefixPath(path);
	std::ifstream config_file(expath.c_str());
	if (!config_file || !config_file.is_open())
		return (NULL);
	std::stringstream stream_binding;
	stream_binding << config_file.rdbuf();
	return (stream_binding.str());
}

int ConfigFile::isFileExistAndReadable(std::string path, std::string const index)
{
	std::string expath = prefixPath(path);
	if (getTypePath(expath + "/" + index) == 1 && checkFile(path + "/" + index, 4) == 0)
		return (0);
	return (-1);
}

bool ConfigFile::isDirectory(std::string& path)
{
	std::string expath = prefixPath(path);
	struct stat statbuf;
	if (stat(expath.c_str(), &statbuf) != 0)
		return false;
	return S_ISDIR(statbuf.st_mode);
}

bool ConfigFile::fileExistsAndReadable(std::string& filePath)
{
	std::string exFilePath = prefixPath(filePath);
	std::ifstream file(exFilePath.c_str());
	bool existsAndReadable = file.good();
	file.close();
	return existsAndReadable;
}
