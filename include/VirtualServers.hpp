/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServers.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 12:41:20 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/22 12:41:20 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VIRTUALSERVERS_HPP
# define VIRTUALSERVERS_HPP

# include "WebServer.hpp"
# include "Location.hpp"
# include "VirtualServers.hpp"
# include "ConfigFile.hpp"

class VirtualServers
{
	private:
		uint16_t				_port;
		in_addr					_ipAddress;
		std::string				_serverName;
		std::string				_root;
		std::string				_index;
		bool					_autoindex;
		std::vector<Location>	_locations;
		unsigned long			_clientMaxBodySize;
		std::string				_return;
		bool					_defaultServer;
		std::map<short, std::string>	_errorPages;
		
		void _createServer(std::string &config, VirtualServers &server);
		bool _checkErrorPages();
	
	public:
		VirtualServers();
		VirtualServers(std::string &config);
		VirtualServers(const VirtualServers &copy);
		~VirtualServers();
		VirtualServers &operator=(const VirtualServers &copy);	
		
		void setPort(std::string parametr);
		void setServerName(std::string parametr);
		void setRoot(std::string parametr);
		void setIndex(std::string parametr);
		void setAutoindex(std::string parametr);
		void setErrorPages(std::vector<std::string> &parametr);
		void setClientMaxBodySize(std::string parametr);
		void setReturn(std::string parametr);
		void setIpAddress(std::string parametr);
		
		const uint16_t &getPort();
		const std::string &getServerName();
		const std::string &getRoot();
		const std::string &getIndex();
		const bool &getAutoindex();
		const std::vector<Location> &getLocations();
		const std::string getErrorPage(int short);
		const std::map<short, std::string> &getErrorPages();
		const unsigned long &getClientMaxBodySize();
		const std::string &getReturn();
		const in_addr &getIpAddress();
		const bool &getDefaultServer();
	
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

#endif // VIRTUALSERVERS_HPP