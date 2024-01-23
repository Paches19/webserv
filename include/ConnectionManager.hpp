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

#ifndef CONNECTION_MANAGER_HPP
# define CONNECTION_MANAGER_HPP

# include <map>
# include <sys/socket.h>
# include <string>
# include <deque>
# include <iostream>
# include <unistd.h>
# include <sstream>
# include "Socket.hpp"
# include "HttpRequest.hpp"
# include "ResponseBuilder.hpp"
# include "ConnectionData.hpp"

class ConnectionManager
{
	private:
		std::map<int, ConnectionData> connections;

	public:
		ConnectionManager();
		~ConnectionManager();

		void addConnection(Socket& socket);
		void removeConnection(Socket& socket);

		void readData(Socket& socket);
		void writeData(Socket& socket);

		bool isHttpRequestComplete(const std::vector<char>& buffer, size_t accumulatedBytes);
		int getContentLength(const std::vector<char>& buffer, size_t accumulatedBytes);
};

#endif // CONNECTION_MANAGER_HPP
