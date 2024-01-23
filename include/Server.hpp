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

# include <iostream>
# include <unistd.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <netinet/in.h>
# include <string>
# include <vector>
# include <map>
# include <poll.h>
# include "Socket.hpp"
# include "ConnectionManager.hpp"
# include "VirtualServers.hpp"

class Server
{
	private:
		std::vector<Socket*> _serverSockets;
		std::map<int, Socket*> _clientSockets;
		ConnectionManager _connectionManager;
		std::vector<struct pollfd> _pollFds;
	
	public:
		Server(std::vector<VirtualServers>	_servers);
		~Server();

		void run();
		// void setPollFds(struct pollfd serverPollFd);
};

#endif // SERVER_HPP
