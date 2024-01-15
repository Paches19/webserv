#ifndef CONFIGFILE_HPP
# define CONFIGFILE_HPP

# include "WebServer.hpp"

class ConfigFile {
	private:
		std::string		_path;
		size_t			_size;
		ConfigFile();
		ConfigFile(const ConfigFile &other);
		ConfigFile &operator=(const ConfigFile &rhs);

	public:		
		ConfigFile(std::string const path);
		~ConfigFile();

		static int checkPath(std::string const path);
		static int checkFile(std::string const path, int mode);
		std::string	readFile(std::string path);

		std::string getPath();
		int getSize();
};

#endif