/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/15 11:33:24 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/06 12:02:00 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

//*******************************************************************
// Constructores y destructor de la clase canónica
//*******************************************************************
Socket::Socket() : _socketFd(-1) {}
Socket::~Socket() {}
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

//*******************************************************************
// Getters
//*******************************************************************
int	Socket::getSocketFd() { return (this->_socketFd); }

sockaddr_in Socket::getSocketAddr() { return (this->_address); }

//*******************************************************************
// Métodos de la clase
//*******************************************************************
bool Socket::open(int port, in_addr addr)
{
	_socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socketFd == -1)
		return false;
	
	int opt = 1;
    if (setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "Error al configurar SO_REUSEADDR" << std::endl;
        return false;
    }
		
	memset(&_address, 0, sizeof(_address));
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = addr.s_addr;
	_address.sin_port = htons(port);

	if (bind(_socketFd, (struct sockaddr *)&_address, sizeof(_address)) < 0)
		return false;

	if (listen(_socketFd, 5) < 0)
		return false;

	return true;
}

bool Socket::accept(Socket& newSocket) const
{
	socklen_t addressLen = sizeof(_address);

	int new_sockfd = ::accept(this->_socketFd, (sockaddr *)&_address, &addressLen);
	if (new_sockfd < 0)
		return false;

	newSocket._socketFd = new_sockfd;
	
	int flags = fcntl(this->_socketFd, F_GETFL, 0);
	return fcntl(_socketFd, F_SETFL, flags | O_NONBLOCK) != -1;
}

int Socket::send(const char* buffer, int length) const
{
	int totalSent = 0;
	int n;

	n = ::send(_socketFd, buffer, length, 0);
	totalSent += n;
	return (n == -1) ? -1 : totalSent;
}

int Socket::receive(char* buffer, int maxLength, size_t startOffset) const
{
	int n = ::recv(_socketFd, buffer + startOffset, maxLength - startOffset, 0);
	if (n > 0) {
		// Lectura exitosa
		std::cout << "Bytes recibidos: " << n << std::endl;
	} else if (n == 0) {
		// Conexión cerrada por el cliente
		std::cout << "Conexion cerrada por cliente" << std::endl;
	} else {
		// Error de recepción
		std::cerr << "Error en la recepcion" << std::endl;
	}
	return n;
}

void Socket::close()
{
	std::cout << "Socket cerrado: " << this->getSocketFd() << std::endl;
	if (_socketFd != -1)
	{
		::close(_socketFd);
		_socketFd = -1;
	}
}
