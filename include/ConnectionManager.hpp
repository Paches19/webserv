/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:42:04 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/22 11:57:32 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTIONMANAGER_HPP
# define CONNECTIONMANAGER_HPP

# include "WebServer.hpp"
# include "Socket.hpp"
# include "VirtualServers.hpp"
# include "ConnectionData.hpp"
# include "HttpRequest.hpp"
# include "ResponseBuilder.hpp"

class ConnectionManager
{
	private:
		std::map<int, ConnectionData> _connections;
		HttpRequest _request;
	
	public:
		ConnectionManager();
		~ConnectionManager();
		ConnectionManager(const ConnectionManager& other);
		ConnectionManager& operator=(const ConnectionManager& other);

		void	addConnection(Socket& socket);
		void	removeConnection(Socket& socket);
		bool	readData(Socket& socket);
		void	writeData(Socket& socket, VirtualServers &_server);
		bool	isHttpRequestComplete(const std::vector<char>& buffer, size_t accumulatedBytes);
		int		getContentLength(const std::vector<char>& buffer, size_t accumulatedBytes);
		HttpRequest&	getRequest();
};

#endif // CONNECTIONMANAGER_HPP
