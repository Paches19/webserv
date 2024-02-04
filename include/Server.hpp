/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:37:38 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/01 18:17:53 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "WebServer.hpp"
# include "Socket.hpp"
# include "ConnectionManager.hpp"
# include "VirtualServers.hpp"
# include "ConfigFile.hpp"

class Server
{
	private:
		std::vector<Socket*> _serverSockets;
		std::vector<Socket*> _clientSockets;
		ConnectionManager _connectionManager;
		std::vector<struct pollfd> _pollFds;
		std::vector<HttpResponse> _responsesToSend;
	
	public:
		Server();
		Server(std::vector<VirtualServers> servers);
		~Server();
		Server(const Server& other);
		Server& operator=(const Server& other);

		std::string getMimeType(const std::string& filePath);
		VirtualServers getBestServer(HttpRequest &request, size_t i, 
			std::vector<VirtualServers> servers);
		
		void run(std::vector<VirtualServers> servers);
		bool areAddressesEqual(const sockaddr_in& addr1, const sockaddr_in& addr2);
		Socket* handleNewConnection(int i);
		void processRequest(HttpRequest request, VirtualServers server, Socket socket);
		std::string buildResourcePath(HttpRequest& request,
			const Location& location, VirtualServers& server);
		std::string adjustPathForDirectory(const std::string& requestURL,
			const std::string& basePath, const Location& location, VirtualServers& server);
		void processReturnDirective(const Location& locationRequest,
			HttpResponse& processResponse);

		class ErrorException : public std::exception
		{
			private:
				std::string _message;
			public:
				ErrorException(std::string message) throw()
				{
					_message = "SERVER ERROR: " + message;
				}
				virtual const char* what() const throw()
				{
					return (_message.c_str());
				}
				virtual ~ErrorException() throw() {}
		};
};

#endif // SERVER_HPP
