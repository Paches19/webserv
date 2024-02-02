/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:38:27 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/01 18:26:06 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server() { }

Server::Server(const Server& other)
{
	_serverSockets = other._serverSockets;
	_clientSockets = other._clientSockets;
	_connectionManager = other._connectionManager;
	_pollFds = other._pollFds;
}

Server& Server::operator=(const Server& other)
{
	if (this != &other)
	{
		_serverSockets = other._serverSockets;
		_clientSockets = other._clientSockets;
		_connectionManager = other._connectionManager;
		_pollFds = other._pollFds;
	}
	return *this;
}

Server::Server(std::vector<VirtualServers>	_servers)
{
	std::cout << "\nInicializando servidor..." << std::endl;
	std::cout << "    Num. servers: " << _servers.size() << std::endl;
	_serverSockets.reserve(_servers.size());
	
	// Crear sockets
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		Socket* newSocket = new Socket();
		if (newSocket->open((int) _servers[i].getPort(), _servers[i].getIpAddress()) == false)
			throw ErrorException("Error al abrir el socket");
			
		_serverSockets.push_back(newSocket);
		
		struct pollfd serverPollFd;

		serverPollFd.fd = _serverSockets[i]->getSocketFd();
		serverPollFd.events = POLLIN; // Establecer para leer
		this->_pollFds.push_back(serverPollFd);
		std::cout << "    Escuchando en el puerto " <<
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

VirtualServers Server::getBestServer(HttpRequest &request, size_t i)
{
	//Busco qué cliente ha hecho la petición para saber qué server le corresponde
	size_t j = 0;
	while (j < _clientSockets.size() && _clientSockets[j]->getSocketFd() != _pollFds[i].fd)
		j++;
	if (j == _clientSockets.size())
		throw ErrorException("Error: no client found");

	int nbServer = 0; //Número de posibles servidores válidos
	int candidates[_servers.size()]; //Array de candidatos a server: 1 = candidato, 0 = no candidato
	long unsigned firstCandidate = _servers.size() - 1; //Posición del primer candidato, en el array de candidatos
	
	//Busco servers con IP:Port del socket cliente = IP:Port del server
	for (long unsigned k = 0; k < _servers.size(); k++)
	{
		if (inet_ntoa(_servers[k].getIpAddress()) == inet_ntoa(_clientSockets[j]->getSocketAddr().sin_addr))
		{
			candidates[k] = 1;
			nbServer++;
			if (firstCandidate > k)
				firstCandidate = k;
		}
		else
			candidates[k] = 0;
	}
	
	// Si no encuentra servers --> ERROR
	if (nbServer == 0)
		throw ErrorException("Error: no server found");
				
	// Si hay varios candidadtos......
	if (nbServer > 1)
	{
		int possibleServers = nbServer;
		// Se busca un server_name coincidente con Host del request
		for (long unsigned k = 0; k < _servers.size(); k++)
		{
			if (candidates[k] == 1 && _servers[k].getServerName() != request.getHost())
				possibleServers--;
			if (candidates[k] == 1 && _servers[k].getServerName() == request.getHost())
			{
				if (firstCandidate > k)
					firstCandidate = k;
			}
		}
		// Si no hay coincidencias, se busca un server_name que tenga
		// coincidencia más larga desde el primer punto hasta el final
		if (possibleServers == 0)
		{	
			possibleServers = nbServer;
			std::string finalServerName;
			std::string finalRequestHost;
			size_t maxLength = 0;
			for (long unsigned k = 0; k < _servers.size(); k++)
			{
				finalServerName = _servers[k].getServerName().substr(_servers[k].getServerName().find_first_of(".") + 1);
				finalRequestHost = request.getHost().substr(request.getHost().find_first_of(".") + 1);
				if (candidates[k] == 1)
				{
					if (finalServerName != finalRequestHost)
						possibleServers--;
					else if (finalServerName.length() > maxLength)
						firstCandidate = k;
				}
			}
		}
		// Si no hay coincidencias, se busca un server_name que tenga
		// coincidencia más larga desde el principio hasta el último punto
		if (possibleServers == 0)
		{
			possibleServers = nbServer;
			std::string inicioServerName;
			std::string inicioRequestHost;
			size_t maxLength = 0;
			for (long unsigned k = 0; k < _servers.size(); k++)
			{
				inicioServerName = _servers[k].getServerName().substr(0, _servers[k].getServerName().find_last_of("."));
				inicioRequestHost = request.getHost().substr(0, request.getHost().find_last_of("."));
				if (candidates[k] == 1)
				{
					if (inicioServerName != inicioRequestHost)
						possibleServers--;
					else if (inicioServerName.length() > maxLength)
						firstCandidate = k;
				}
			}
		}
		// Si no hay coincidencias, se busca un server_name definido 
		// con expresiones regulares (~ antes del nombre) que coincida
		// con el nombre del Host del request
		if (possibleServers == 0)
		{
			possibleServers = nbServer;

		}
		//Si no hay coincidencias, se usa el predeterminado:
		// Este server es el primero del archivo de configuración (si default_server no está definido)
		// o el que tenga la directiva default_server (solo puede haber uno)
		if (possibleServers == 0)
		{
			possibleServers = nbServer;
			for (long unsigned k = 0; k < _servers.size(); k++)
			{
				if (candidates[k] == 1)
				{
					if (!_servers[k].getDefaultServer())
						possibleServers--;
					else
					{
						firstCandidate = k;
						break ;
					}
				}
			}
		}
		//Si no hay coincidencias, se usa el primer server de la lista
		if (possibleServers == 0)
		{
			possibleServers = nbServer;
			for (long unsigned k = 0; k < _servers.size(); k++)
			{
				if (candidates[k] == 1)
				{
					firstCandidate = k;
					break;
				}
			}
		}
	}
	return _servers[firstCandidate];
}

void Server::run(std::vector<VirtualServers> _servers)
{
	std::cout << "\nServidor en ejecución..." << std::endl;

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
			HttpRequest requestReceive;
			VirtualServers bestServer;

			if (ret >= 0 && _pollFds[i].revents & POLLIN)
			{
				Socket* dataSocket = handleNewConnection(i);
				if (dataSocket && dataSocket->getSocketFd() != -1 &&
					_pollFds[i].fd == dataSocket->getSocketFd())
				{
					requestReceive = _connectionManager.readData(*dataSocket);
					bestServer = getBestServer(requestReceive, i);
					_pollFds[i].events = POLLOUT;

					/*if (!_connectionManager.readData(*dataSocket))
					{
						int currentFd = _pollFds[i].fd;
						for (size_t j = 0; j < _clientSockets.size(); ++j)
						{
							if (_clientSockets[j]->getSocketFd() == currentFd)
							{
								std::cout << "Client socket erased " << _clientSockets[j]->getSocketFd() << std::endl;
								_clientSockets.erase(_clientSockets.begin() + j);
							}
						}
						_pollFds.erase(_pollFds.begin() + i);
						--i;
						_connectionManager.removeConnection(*dataSocket);
					}
					else
						_pollFds[i].events = POLLOUT;*/
					
				}
			}
			else if ((_pollFds[i].revents & POLLOUT))
			{
				for (size_t j = 0; j < _clientSockets.size(); ++j)
				{
					if (_clientSockets[j]->getSocketFd() == _pollFds[i].fd)
					{
						_connectionManager.writeData(*(_clientSockets[j]), bestServer);
						_pollFds[i].events = POLLIN;
						break ;
					}
				}
			}
			else if (_pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				// Manejar desconexiones o errores
				std::cout << "Conexión cerrada o error en el socket FD: " << _pollFds[i].fd << std::endl;
				int currentFd = _pollFds[i].fd;
				for (size_t j = 0; j < _clientSockets.size(); ++j)
				{
					if (_clientSockets[j]->getSocketFd() == currentFd)
					{
						std::cout << "Client socket erased " << _clientSockets[j]->getSocketFd() << std::endl;
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
			std::cout << "    Nueva conexión";
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
			std::cerr << "Error al aceptar nueva conexión" << std::endl;
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
