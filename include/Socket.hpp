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

# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <iostream>
#include <errno.h>

class Socket
{
	private:
		int _socketFd;
		Socket(Socket& other);
		Socket& operator=(Socket& other);

	public:
		Socket();
		~Socket();

		bool open(int port);
		bool accept(Socket& newSocket) const;
		int send(const char* buffer, int length) const;
		int receive(char* buffer, int length) const;
		void close();
		int	getSocketFd();
};

#endif // SOCKET_HPP