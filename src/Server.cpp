/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:38:27 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/22 13:43:16 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

Server::Server(std::vector<VirtualServers>	_servers)
{
	std::cout << "Inicializando servidor..." << std::endl;
	std::cout << "Servers: " << _servers.size() << std::endl;
	_serverSockets.reserve(_servers.size());
	// Crear sockets
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		std::cout << "For i: " << i << std::endl;
		_serverSockets[i] = new Socket;
		if (_serverSockets[i]->open((int) _servers[i].getPort()) == false)
		{
			std::cerr << "Error al abrir el socket" << std::endl;
			return ;
		}
		std::cout << "Return true " << std::endl;
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
        delete _serverSockets[i];
}

void Server::run()
{
	std::cout << "Servidor en ejecución..." << std::endl;

	while (true)
	{
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
				if (i < _serverSockets.size())
				{
					Socket* newSocket = new Socket();
					if (_serverSockets[i]->accept(*newSocket))
					{
						struct pollfd newPollFd;
						newPollFd.fd = newSocket->getSocketFd();
						newPollFd.events = POLLIN | POLLOUT; // Listo para leer y escribir
						_pollFds.push_back(newPollFd);
						_connectionManager.addConnection(*newSocket);
						_serverSockets.push_back(newSocket);
					}
					else
					{
						delete newSocket;
						std::cerr << "Error al aceptar nueva conexión" << std::endl;
					}
				}
				else
					_connectionManager.readData(*_serverSockets[i]);
			}
			else if (_pollFds[i].revents & POLLOUT)
				_connectionManager.writeData(*_serverSockets[i]);
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
