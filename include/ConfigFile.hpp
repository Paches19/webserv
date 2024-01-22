#ifndef CONFIGFILE_HPP
# define CONFIGFILE_HPP

# include "WebServer.hpp"

class ConfigFile {
	private:
		std::string	_path;
		ConfigFile();
		ConfigFile(const ConfigFile &other);
		ConfigFile &operator=(const ConfigFile &rhs);

	public:		
		ConfigFile(std::string const path);
		~ConfigFile();

		static int checkPath(std::string const path);
		static int checkFile(std::string const path, int mode);
		std::string	readFile(std::string path);
		static int isFileExistAndReadable(std::string const path, std::string const index);
		std::string getPath();
		static int getTypePath(std::string const path);
};

#endif