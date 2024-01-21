#ifndef VIRTUALSERVERS_HPP
# define VIRTUALSERVERS_HPP

# include "WebServer.hpp"

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
		
		VirtualServers();
		void _createServer(std::string &config, VirtualServers &server);
		bool _checkErrorPages();
	
	public:	
		VirtualServers &operator=(const VirtualServers &copy);	
		VirtualServers(std::string &config);
		VirtualServers(const VirtualServers &copy);
		~VirtualServers();
		// Setters
		void setPort(std::string parametr);
		void setServerName(std::string server_name);
		void setRoot(std::string root);
		void setIndex(std::string index);
		void setAutoindex(std::string autoindex);
		void setErrorPages(std::vector<std::string> &parametr);
		void setClientMaxBodySize(std::string parametr);
		void setReturn(std::string parametr);
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