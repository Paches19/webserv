/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:37:38 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/23 11:08:35 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "WebServer.hpp"
# include "Socket.hpp"
# include "ConnectionManager.hpp"
# include "VirtualServers.hpp"

class Server
{
	private:
		std::vector<Socket*> _serverSockets;
		std::vector<Socket*> _clientSockets;
		ConnectionManager _connectionManager;
		std::vector<struct pollfd> _pollFds;
	
	public:
		Server(std::vector<VirtualServers> _servers);
		~Server();

		void run(std::vector<VirtualServers> _servers);
		
		bool areAddressesEqual(const sockaddr_in& addr1, const sockaddr_in& addr2);
		Socket* handleNewConnection(int i);
};

#endif // SERVER_HPP
