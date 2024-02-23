/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 12:40:57 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/23 18:10:55 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include "WebServer.hpp"
# include "VirtualServers.hpp"
# include "ConfigFile.hpp"

class ConfigParser
{
	private:
		std::vector<VirtualServers>	_servers;
		std::vector<std::string>	_serverConfig;
		size_t						_nbServer;
		
	public:
		ConfigParser();
		~ConfigParser();
		ConfigParser(const ConfigParser &other);
		ConfigParser &operator=(const ConfigParser &rhs);

		std::vector<VirtualServers> getServers();

		int		initParser(const std::string &configFile);
		void	splitServers(std::string &content);
		
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

#endif //CONFIGPARSER_HPP