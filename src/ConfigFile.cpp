#include "../include/ConfigFile.hpp"

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

int ConfigFile::checkPath(std::string const path)
{
	struct stat	buffer;
	int			result;
	
	result = stat(path.c_str(), &buffer);
	if (result == 0)
			return (1);
	return (-1);
}

// Check is the file exists and accessable
int	ConfigFile::checkFile(std::string const path, int mode)
{
	return (access(path.c_str(), mode));
}

// Read from file to string
std::string	ConfigFile::readFile(std::string path)
{
	if (path.empty() || path.length() == 0)
		return (NULL);
	std::ifstream configFile(path.c_str());
	if (!configFile || !configFile.is_open())
		return (NULL);
	std::stringstream streamBinding;
	streamBinding << configFile.rdbuf();
	return (streamBinding.str());
}

std::string ConfigFile::getPath()
{
	return (_path);
}

// Get if path is file(1), folder(2) or something else(3)
int ConfigFile::getTypePath(std::string const path)
{
	struct stat	buffer;

	if (stat(path.c_str(), &buffer) == 0)
	{
		if (buffer.st_mode & S_IFREG)
			return (1);
		if (buffer.st_mode & S_IFDIR)
			return (2);
		return (3);
	}
	return (-1);
}

int ConfigFile::isFileExistAndReadable(std::string const path, std::string const index)
{
	if (getTypePath(index) == 1 && checkFile(index, 4) == 0)
		return (0);
	if (getTypePath(path + index) == 1 && checkFile(path + index, 4) == 0)
		return (0);
	return (-1);
}
