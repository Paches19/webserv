#ifndef VIRTUALSERVERS_HPP
# define VIRTUALSERVERS_HPP

# include "WebServer.hpp"

static std::string	serverParametrs[] = {"server_name", "listen", "root", "index", "allow_methods"};

class Location;

class VirtualServers
{
	private:
		uint16_t				_listen;
		std::string				_server_name;
		std::string				_root;
		std::string				_index;
		bool					_autoindex;
		std::vector<Location>	_locations;
		std::map<short, std::string>	_error_pages;
		unsigned long			_client_max_body_size;
		std::string				_return;

        struct sockaddr_in 		_server_address;
        int     				_listen_fd;
		
		VirtualServers();

		void _createServer(std::string &config, VirtualServers &server);
		bool _isValidErrorPages();
		void _checkToken(std::string &parametr);
		int _isValidLocation(Location &location) const;
		bool _checkLocations() const;
	public:	
		VirtualServers(std::string &config);
		~VirtualServers();
		VirtualServers(const VirtualServers &copy);
		VirtualServers &operator=(const VirtualServers &copy);
		static int ft_stoi(std::string str);
		// Setters
		void setPort(std::string parametr);
		void setServerName(std::string server_name);
		void setRoot(std::string root);
		void setIndex(std::string index);
		void setAutoindex(std::string autoindex);
		void setLocation(std::string nameLocation, std::vector<std::string> parametr);
		void setErrorPages(std::vector<std::string> &parametr);
		void setClientMaxBodySize(std::string parametr);
		void setReturn(std::string parametr);
		void setServerAddress(struct sockaddr_in server_address);
		void setFd(int);

		// Getters
		const uint16_t &getPort();
		const std::string &getServerName();
		const std::string &getRoot();
		const std::string &getIndex();
		const bool &getAutoindex();
		const std::vector<Location> &getLocations();
		const std::map<short, std::string> &getErrorPages();
		const unsigned long &getClientMaxBodySize();
		const std::string &getReturn();
		const struct sockaddr_in &getServerAddress();
		const int &getFd();
		const std::string &getPathErrorPage(short key);
		const std::vector<Location>::iterator getLocationKey(std::string key);
	
		class ErrorException : public std::exception
		{
			private:
				std::string _message;
			public:
				ErrorException(std::string message) throw()
				{
					_message = "CONFIG SERVER ERROR: " + message;
				}
				virtual const char* what() const throw()
				{
					return (_message.c_str());
				}
				virtual ~ErrorException() throw() {}
		};
};

#endif