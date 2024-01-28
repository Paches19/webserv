/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:38:27 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/24 13:01:17 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(std::vector<VirtualServers>	_servers)
{
	std::cout << "\n****Inicializando servidor..." << std::endl;
	std::cout << "    Servers: " << _servers.size() << std::endl;
	_serverSockets.reserve(_servers.size());
	
	// Crear sockets
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		Socket* newSocket = new Socket();
		if (newSocket->open((int) _servers[i].getPort(), _servers[i].getIpAddress()) == false)
		{
			std::cerr << "    Error al abrir el socket" << std::endl;
			return ;
		}
		_serverSockets.push_back(newSocket);
		
		struct pollfd serverPollFd;

		serverPollFd.fd = _serverSockets[i]->getSocketFd();
		serverPollFd.events = POLLIN; // Establecer para leer
		this->_pollFds.push_back(serverPollFd);
		std::cout << "    Servidor escuchando en el puerto " <<
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

void Server::run(std::vector<VirtualServers> _servers)
{
	std::cout << "\n****Servidor en ejecución..." << std::endl;

	while (true)
	{
		fd_set readfds;
		for (size_t i = 0; i < _pollFds.size(); ++i)
			FD_SET(_pollFds[i].fd, &readfds);

		// Llamar a poll con la lista de file descriptors y un tiempo de espera
		int ret = poll(&_pollFds[0], _pollFds.size(), -1); // -1 para tiempo de espera indefinido

		if (ret < 0)
		{
			std::cerr << "    Error en poll" << std::endl;
			break;
		}

		// Revisar si hay nuevas conexiones en el socket del servidor
		for (size_t i = 0; i < _pollFds.size(); ++i)
		{
			if (ret >= 0 && _pollFds[i].revents & POLLIN)
			{
				Socket* dataSocket = handleNewConnection(i);
				if (dataSocket && dataSocket->getSocketFd() != -1 &&
					_pollFds[i].fd == dataSocket->getSocketFd())
				{
					if (!_connectionManager.readData(*dataSocket))
					{
						int currentFd = _pollFds[i].fd;
						for (size_t j = 0; j < _clientSockets.size(); ++j)
						{
							if (_clientSockets[j]->getSocketFd() == currentFd)
							{
								std::cout << "    Client socket erase" << _clientSockets[j]->getSocketFd() << std::endl;
								_clientSockets.erase(_clientSockets.begin() + j);
							}
						}
						_pollFds.erase(_pollFds.begin() + i);
						--i;
						_connectionManager.removeConnection(*dataSocket);
					}
					else
						_pollFds[i].events = POLLOUT;
					std::cout << "    Salgo readData " << std::endl;
				}
			}
			else if ((_pollFds[i].revents & POLLOUT))
			{
				std::cout << "\n****Handle Request " << std::endl;
				std::cout << "    socketFd " << _pollFds[i].fd << std::endl;
				int currentFd = _pollFds[i].fd;
				for (size_t j = 0; j < _clientSockets.size(); ++j)
				{
					if (_clientSockets[j]->getSocketFd() == currentFd)
					{
						int nb_server = 0;
						for (long unsigned k = 0; k < _servers.size(); k++)
						{
							if (inet_ntoa(_servers[k].getIpAddress()) == inet_ntoa(_clientSockets[j]->getSocketAddr().sin_addr))
							{
								nb_server = k;
								break;
							}
						}
						std::cout << "    Writing Data from server " << nb_server << std::endl;
						_connectionManager.writeData(*(_clientSockets[j]), _servers[nb_server]);
						_pollFds[i].events = POLLIN;
						break ;
					}
				}
			}
			else if (_pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				// Manejar desconexiones o errores
				std::cout << "\n****Conexión cerrada o error en el socket FD: " << _pollFds[i].fd << std::endl;
				int currentFd = _pollFds[i].fd;
				for (size_t j = 0; j < _clientSockets.size(); ++j)
				{
					if (_clientSockets[j]->getSocketFd() == currentFd)
					{
						std::cout << "    Client socket erase" << _clientSockets[j]->getSocketFd() << std::endl;
						_clientSockets.erase(_clientSockets.begin() + j);
						_connectionManager.removeConnection(*(_clientSockets[j]));
					}
				}
				_pollFds.erase(_pollFds.begin() + i);
				--i;
			}
		}
	}
}

bool Server::areAddressesEqual(const sockaddr_in& addr1, const sockaddr_in& addr2)
{
	return (addr1.sin_addr.s_addr == addr2.sin_addr.s_addr) &&
		(addr1.sin_port == addr2.sin_port);
}

Socket* Server::handleNewConnection(int i)
{
	Socket* existingSocket;
	if (i < (int) _serverSockets.size())
	{
		Socket* newSocket = new Socket();
		if (_serverSockets[i]->accept(*newSocket))
		{
			std::vector<Socket*>::iterator it;
			for (it = _clientSockets.begin(); it != _clientSockets.end(); ++it)
			{
				existingSocket = *it;
				if (areAddressesEqual(newSocket->getSocketAddr(), existingSocket->getSocketAddr()))
				{
					std::cout << "    Cliente existente" << std::endl;
					delete newSocket;
					return existingSocket;
				}
			}
			std::cout << "    Nueva conexion" << std::endl;
			struct pollfd newPollFd;
			newPollFd.fd = newSocket->getSocketFd();
			newPollFd.events = POLLIN | POLLOUT;
			_connectionManager.addConnection(*newSocket);
			_pollFds.push_back(newPollFd);
			_clientSockets.push_back(newSocket);
			return newSocket;
		}
		else
		{
			delete newSocket;
			std::cerr << "    Error al aceptar nueva conexión" << std::endl;
			Socket *errorSocket = NULL;
			return errorSocket;
		}
	}
	else
	{
		for (size_t j = 0; j < _clientSockets.size(); ++j)
		{
			if (_clientSockets[j]->getSocketFd() == _pollFds[i].fd)
				return (_clientSockets[j]);
		}
	}
	return _serverSockets[0];
}
