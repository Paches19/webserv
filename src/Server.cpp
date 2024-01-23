/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:38:27 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/23 12:34:42 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(std::vector<VirtualServers>	_servers)
{
	std::cout << "Inicializando servidor..." << std::endl;
	std::cout << "Servers: " << _servers.size() << std::endl;
	_serverSockets.reserve(_servers.size());
	
	// Crear sockets
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		Socket* newSocket = new Socket();
		if (newSocket->open((int) _servers[i].getPort()) == false)
		{
			std::cerr << "Error al abrir el socket" << std::endl;
			return ;
		}
		_serverSockets.push_back(newSocket);
		
		struct pollfd serverPollFd;

		serverPollFd.fd = _serverSockets[i]->getSocketFd();
		serverPollFd.events = POLLIN; // Establecer para leer
		this->_pollFds.push_back(serverPollFd);
		std::cout << "Servidor escuchando en el puerto " <<
		_servers[i].getPort() << std::endl;
	}
}

Server::~Server()
{
	for (size_t i = 0; i < _serverSockets.size(); ++i)
	{
		_serverSockets[i]->close();
		delete _serverSockets[i];
	}
	
	for (size_t i = 0; i < _clientSockets.size(); ++i)
	{
		_clientSockets[i]->close();
		delete _clientSockets[i];
	}
}

void Server::run()
{
	std::cout << "Servidor en ejecución..." << std::endl;

	while (true)
	{
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds);
		for (size_t i = 0; i < _pollFds.size(); ++i)
			FD_SET(_pollFds[i].fd, &readfds);

		// Llamar a poll con la lista de file descriptors y un tiempo de espera
		int ret = poll(&_pollFds[0], _pollFds.size(), -1); // -1 para tiempo de espera indefinido

		if (ret < 0)
		{
			std::cerr << "Error en poll" << std::endl;
			break;
		}

		// Revisar si hay nuevas conexiones en el socket del servidor
		for (size_t i = 0; i < _pollFds.size(); ++i)
		{
			if (_pollFds[i].revents & POLLIN)
			{
				std::cout << "i: " << i << std::endl;
				std::cout << "_serverSockets.size(): " << _serverSockets.size() << std::endl;
				std::cout << "_clientSockets.size(): " << _clientSockets.size() << std::endl;
				if (i < _serverSockets.size())
				{
					Socket* newSocket = new Socket();			
					if (_serverSockets[i]->accept(*newSocket))
					{
						std::cout << "Nuevo socket con fd: " << newSocket->getSocketFd() << std::endl;
						struct pollfd newPollFd;
						newPollFd.fd = newSocket->getSocketFd();
						newPollFd.events = POLLIN | POLLOUT;
						_pollFds.push_back(newPollFd);
						_connectionManager.addConnection(*newSocket);
						int clientKey = i + _serverSockets.size() + _clientSockets.size();
						_clientSockets.insert(std::make_pair(clientKey, newSocket));
					}
					else
					{
						delete newSocket;
						std::cerr << "Error al aceptar nueva conexión" << std::endl;
					}
				}
				else
				{
					if (_clientSockets.find(i) != _clientSockets.end())
					{
						std::cout << "Llegada de datos " << std::endl;
    					_connectionManager.readData(*_clientSockets[i]);
					}
					else
						std::cout << "Llegada de datos sin clientSocket" << std::endl;
				}
			}
			else if (_pollFds[i].revents & POLLOUT)
				_connectionManager.writeData(*_clientSockets[i]);
			else if (_pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				// Manejar desconexiones o errores
				std::cout << "Conexión cerrada o error en el socket FD: " << _pollFds[i].fd << std::endl;
				_connectionManager.removeConnection(*_serverSockets[i]);
				_pollFds.erase(_pollFds.begin() + i);
				--i; // Ajustar el índice después de borrar un elemento
			}
		}
	}
}
