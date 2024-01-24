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
				// std::cout << "i: " << i << std::endl;
				// std::cout << "_serverSockets.size(): " << _serverSockets.size() << std::endl;
				// std::cout << "_clientSockets.size(): " << _clientSockets.size() << std::endl;
				Socket* dataSocket = handleNewConnection(_serverSockets[i]);
				if (dataSocket)
				{
					std::cout << "Llegada de datos " << std::endl;
					_connectionManager.readData(*dataSocket);
				}
				// else
				// 	std::cout << "Llegada de datos sin clientSocket" << std::endl;
			}
			else if (_pollFds[i].revents & POLLOUT)
			{
				std::cout << "Handle Request " << std::endl;
				handleRequest(*_clientSockets[i]);
			}
			else if (_pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				// Manejar desconexiones o errores
				std::cout << "Conexión cerrada o error en el socket FD: " << _pollFds[i].fd << std::endl;
				_connectionManager.removeConnection(*_serverSockets[i]);
				// _pollFds.erase(_pollFds.begin() + i);
				// --i; // Ajustar el índice después de borrar un elemento
			}
		}
	}
}


bool Server::areAddressesEqual(const sockaddr_in& addr1, const sockaddr_in& addr2)
{
	return (addr1.sin_addr.s_addr == addr2.sin_addr.s_addr) &&
		(addr1.sin_port == addr2.sin_port);
}

Socket* Server::handleNewConnection(Socket* serverSocket)
{
	Socket* newSocket = new Socket();
	Socket* existingSocket;
	if (serverSocket->accept(*newSocket))
	{
		bool isExistingClient = false;
		std::map<int, Socket*>::iterator it;
		for (it = _clientSockets.begin(); it != _clientSockets.end(); ++it)
		{
			existingSocket = it->second;
			if (areAddressesEqual(newSocket->getSocketAddr(), existingSocket->getSocketAddr()))
			{
				std::cout << "Cliente existente" << std::endl;
				delete newSocket;
				isExistingClient = true;
				return existingSocket;
			}
		}
		if (!isExistingClient)
		{
			std::cout << "Nueva conexion" << std::endl;
			struct pollfd newPollFd;
			newPollFd.fd = newSocket->getSocketFd();
			newPollFd.events = POLLIN | POLLOUT;
			_connectionManager.addConnection(*newSocket);
			_pollFds.push_back(newPollFd);
			int clientKey = _pollFds.size();
			_clientSockets.insert(std::make_pair(clientKey, newSocket));
			return newSocket;
		}
		return existingSocket;
	}
	else
	{
		delete newSocket;
		std::cerr << "Error al aceptar nueva conexión" << std::endl;
		Socket *errorSocket = NULL;
		return errorSocket;
	}
}

void Server::handleRequest(Socket& clientSocket)
{
    ResponseBuilder responseBuilder;

    // Configurar la respuesta
	std::cout << "Respondiendo" << std::endl;
    responseBuilder.setStatusCode(200);
    responseBuilder.addHeader("Content-Type", "text/html");
    responseBuilder.setBody("<html><body><h1>Hello, World!</h1></body></html>");

    // Construir la respuesta
    std::string response = responseBuilder.buildResponse();

    // Enviar la respuesta
    clientSocket.send(response.c_str(), response.size());
	std::cout << "Respuesta enviada" << std::endl;
}
