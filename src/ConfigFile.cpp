#include "../include/ConfigFile.hpp"

ConfigFile::ConfigFile() : _size(0) { }

ConfigFile::ConfigFile(const ConfigFile &other)
{
	this->_path = other._path;
	this->_size = other._size;
}

ConfigFile &ConfigFile::operator=(const ConfigFile &rhs)
{
	if (this == &rhs)
		return (*this);
	this->_path = rhs._path;
	this->_size = rhs._size;
	return (*this);
}

ConfigFile::ConfigFile(std::string const path) : _path(path), _size(0) { }

ConfigFile::~ConfigFile() { }

/* define is path is file(1), folder(2) or something else(3) */
int ConfigFile::checkPath(std::string const path)
{
	struct stat	buffer;
	int			result;
	
	result = stat(path.c_str(), &buffer);
	if (result == 0 && (buffer.st_mode & S_IFREG))
			return (1);
	return (-1);
}

/* checks is the file exists and accessable */
int	ConfigFile::checkFile(std::string const path, int mode)
{
	return (access(path.c_str(), mode));
}

/* reading from file to string */
std::string	ConfigFile::readFile(std::string path)
{
	if (path.empty() || path.length() == 0)
		return (NULL);
	std::ifstream config_file(path.c_str());
	if (!config_file || !config_file.is_open())
		return (NULL);

	std::stringstream stream_binding;
	stream_binding << config_file.rdbuf();
	return (stream_binding.str());
}

/* Get functions */
std::string ConfigFile::getPath()
{
	return (this->_path);
}

int ConfigFile::getSize()
{
	return (this->_size);
}
