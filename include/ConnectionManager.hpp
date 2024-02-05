/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:42:04 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/05 12:59:08 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTIONMANAGER_HPP
# define CONNECTIONMANAGER_HPP

# include "WebServer.hpp"
# include "Socket.hpp"
# include "VirtualServers.hpp"
# include "ConnectionData.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"

class ConnectionManager
{
	public:
		std::map<int, ConnectionData> connections;
		ConnectionManager();
		~ConnectionManager();
		ConnectionManager(const ConnectionManager& other);
		ConnectionManager& operator=(const ConnectionManager& other);

		void	addConnection(Socket& socket);
		void	removeConnection(Socket& socket, int i,
			std::vector<struct pollfd> &_pollFds, std::vector<Socket *> &_clientSockets);
		HttpRequest	readData(Socket& socket, int i,
			std::vector<struct pollfd> &_pollFds, std::vector<Socket *> &_clientSockets);
		void	writeData(Socket& socket,  int i,
			HttpResponse &response, std::vector<struct pollfd> &_pollFds);
		bool	isHttpRequestComplete(const std::vector<char>& buffer, size_t accumulatedBytes);
		int		getContentLength(const std::vector<char>& buffer, size_t accumulatedBytes);
		
		
};

#endif // CONNECTIONMANAGER_HPP
