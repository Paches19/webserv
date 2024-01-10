/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   connectionManager.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:42:04 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/10 12:46:02 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_MANAGER_HPP
# define CONNECTION_MANAGER_HPP

# include <map>
# include <sys/socket.h>
# include <string>
# include <iostream>
# include <unistd.h>

class ConnectionManager
{
	private:
		std::map<int, std::string> connections;  // Mapa de descriptores de socket
	
	public:
		ConnectionManager();
		~ConnectionManager();

		void addConnection(int socketFd);
		void removeConnection(int socketFd);
		void processData(int socketFd);
};

#endif // CONNECTION_MANAGER_HPP
