/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/15 11:33:24 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/22 13:46:20 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket() : _socketFd(-1) {}

Socket::~Socket()
{
	// if (_socketFd != -1)
	// 	::close(_socketFd);
}

Socket::Socket(Socket& other) : _socketFd(other._socketFd)
{
	other._socketFd = -1;
}

Socket& Socket::operator=(Socket& other)
{
	if (this != &other)
	{
		if (_socketFd != -1)
			::close(_socketFd);
		_socketFd = other._socketFd;
		other._socketFd = -1;
	}
	return *this;
}

bool Socket::open(int port)
{
	std::cout << "Open port: " << port << std::endl;
	_socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socketFd == -1)
		return false;
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(_socketFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
		return false;

	if (listen(_socketFd, 5) < 0)
		return false;

	return true;
}

bool Socket::accept(Socket& newSocket) const
{
	sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int new_sockfd = ::accept(_socketFd, (sockaddr *)&client_addr, &client_len);
	if (new_sockfd < 0)
		return false;

	newSocket._socketFd = new_sockfd;
	return true;
}

int Socket::send(const char* buffer, int length) const
{
	int totalSent = 0;
	int bytesLeft = length;
	int n;

	while (totalSent < length)
	{
		n = ::send(_socketFd, buffer + totalSent, bytesLeft, 0);
		if (n == -1)
			break;
		totalSent += n;
		bytesLeft -= n;
	}

	return (n == -1) ? -1 : totalSent;
}

int Socket::receive(char* buffer, int length) const
{
	int totalReceived = 0;
	int n;

	while (totalReceived < length)
	{
		n = ::recv(_socketFd, buffer + totalReceived, length - totalReceived, 0);
		if (n == -1 || n == 0)
			break;
		totalReceived += n;
	}

	return (n <= 0) ? -1 : totalReceived;
}

void Socket::close()
{
	if (_socketFd != -1)
	{
		::close(_socketFd);
		_socketFd = -1;
	}
}

int	Socket::getSocketFd()
{
	return (this->_socketFd);
}