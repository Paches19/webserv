#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include "WebServer.hpp"

class VirtualServers;

class ConfigParser
{
	private:
		std::vector<VirtualServers>	_servers;
		std::vector<std::string>	_serverConfig;
		size_t						_nbServer;
		ConfigParser(const ConfigParser &other);
		ConfigParser &operator=(const ConfigParser &rhs);

	public:
		ConfigParser();
		~ConfigParser();

		int initParser(const std::string &configFile);
		void splitServers(std::string &content);
		std::vector<VirtualServers> &getServers();
		int print();

		class ErrorException : public std::exception
		{
			private:
				std::string _message;
			public:
				ErrorException(std::string message) throw()
				{
					_message = "CONFIG PARSER ERROR: " + message;
				}
				virtual const char* what() const throw()
				{
					return (_message.c_str());
				}
				virtual ~ErrorException() throw() {}
		};
};

#endif