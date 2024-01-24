/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/15 11:32:14 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/22 17:01:59 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "WebServer.hpp"

class Socket
{
	private:
		int _socketFd;
		sockaddr_in _address;

	public:
		Socket();
		~Socket();
		Socket(Socket& other);
		Socket& operator=(Socket& other);

		bool open(int port, in_addr addr);
		bool accept(Socket& newSocket) const;
		int send(const char* buffer, int length) const;
		int receive(char* buffer, int length) const;
		void close();
		
		int	getSocketFd();
		sockaddr_in	getSocketAddr();
};

#endif // SOCKET_HPP