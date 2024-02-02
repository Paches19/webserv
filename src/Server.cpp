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

Server::Server(std::vector<VirtualServers>	servers)
{
	std::cout << "\nInicializando servidor..." << std::endl;
	std::cout << "    Num. servers: " << servers.size() << std::endl;
	_serverSockets.reserve(servers.size());
	
	// Crear sockets
	for (size_t i = 0; i < servers.size(); ++i)
	{
		Socket* newSocket = new Socket();
		if (newSocket->open((int) servers[i].getPort(), servers[i].getIpAddress()) == false)
			throw ErrorException("Error al abrir el socket");
			
		_serverSockets.push_back(newSocket);
		
		struct pollfd serverPollFd;

		serverPollFd.fd = _serverSockets[i]->getSocketFd();
		serverPollFd.events = POLLIN; // Establecer para leer
		this->_pollFds.push_back(serverPollFd);
		std::cout << "    Escuchando en el puerto " <<
		servers[i].getPort() << std::endl;
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

VirtualServers Server::getBestServer(HttpRequest &request, size_t i, std::vector<VirtualServers> servers)
{
	//Busco qué cliente ha hecho la petición para saber qué server le corresponde
	size_t j = 0;

	while (j < _clientSockets.size() && _clientSockets[j]->getSocketFd() != _pollFds[i].fd)
		j++;
	if (j == _clientSockets.size())
		throw ErrorException("Error: no client found");

	int nbServer = 0; //Número de posibles servidores válidos
	int candidates[servers.size()]; //Array de candidatos a server: 1 = candidato, 0 = no candidato
	long unsigned firstCandidate = servers.size() - 1; //Posición del primer candidato, en el array de candidatos
	
	//Busco servers con IP:Port del socket cliente = IP:Port del server
	for (long unsigned k = 0; k < servers.size(); k++)
	{
		if (inet_ntoa(servers[k].getIpAddress()) == inet_ntoa(_clientSockets[j]->getSocketAddr().sin_addr))
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
		for (long unsigned k = 0; k < servers.size(); k++)
		{
			if (candidates[k] == 1 && servers[k].getServerName() != request.getHost())
				possibleServers--;
			if (candidates[k] == 1 && servers[k].getServerName() == request.getHost())
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
			for (long unsigned k = 0; k < servers.size(); k++)
			{
				finalServerName = servers[k].getServerName().substr(servers[k].getServerName().find_first_of(".") + 1);
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
			for (long unsigned k = 0; k < servers.size(); k++)
			{
				inicioServerName = servers[k].getServerName().substr(0, servers[k].getServerName().find_last_of("."));
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
		//**********************************************************
		// Si no hay coincidencias, se busca un server_name definido 
		// con expresiones regulares (~ antes del nombre) que coincida
		// con el nombre del Host del request 
		// ESTA PARTE NO SE IMPLEMENTA SEGÚN SUBJECT
		//***********************************************************
		// Si no hay coincidencias, se usa el predeterminado.
		// Este server es el que tenga la directiva default_server (solo puede haber uno)
		if (possibleServers == 0)
		{
			possibleServers = nbServer;
			for (long unsigned k = 0; k < servers.size(); k++)
			{
				if (candidates[k] == 1)
				{
					if (!servers[k].getDefaultServer())
						possibleServers--;
					else
					{
						firstCandidate = k;
						break ;
					}
				}
			}
		}
		// Y Si no está definido default_server, se usa el primer server de la lista
		if (possibleServers == 0)
		{
			possibleServers = nbServer;
			for (long unsigned k = 0; k < servers.size(); k++)
			{
				if (candidates[k] == 1)
				{
					firstCandidate = k;
					break;
				}
			}
		}
	}
	return servers[firstCandidate];
}

void Server::run(std::vector<VirtualServers> servers)
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
					requestReceive = _connectionManager.readData(*dataSocket, i, _pollFds, _clientSockets);
					if (requestReceive.isValidRequest() && requestReceive.isCompleteRequest())
					{
						bestServer = getBestServer(requestReceive, i, _servers);
						processRequest(requestReceive, bestServer, *dataSocket);				
					}
					else if (!requestReceive.isValidRequest())
					{
						--i;
					}	
				}
			}
			else if ((_pollFds[i].revents & POLLOUT))
			{
				for (size_t j = 0; j < _clientSockets.size(); ++j)
				{
					if (_clientSockets[j]->getSocketFd() == _pollFds[i].fd)
					{
						_connectionManager.writeData(*(_clientSockets[j]), bestServer, requestReceive);
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
				};
			}
		}
	}
}

void Server::processRequest(HttpRequest request, VirtualServers server, Socket socket)
{
	ConnectionData& data(_connectionManager.connections[socket.getSocketFd()]);

	if (data.responseSent)
		return;
	
	HttpResponse processResponse;

	// Configurar la respuesta
	std::cout << "\nProcesando REQUEST: " << request.getMethod() << std::endl;
	std::cout << "    Searching for URL: " << request.getUri() << std::endl;
	std::string frontpage = server.getRoot() + server.getIndex();
	
	std::vector<Location> locations = server.getLocations();
	const Location*	locationRequest = nullptr;
	
	if (!locations.empty())
		locationRequest = locations[0].selectLocation(request.getUri(), locations);
	if (locationRequest == nullptr)
	{
		processResponse.setStatusCode(404);
		processResponse.setBody("404 Not Found");
		_responsesToSend[socket.getSocketFd()] = processResponse;
		return ;
	}
	if (request.getMethod() == "GET")
	{
		//CONSTRUIMOS RUTA DEL ARCHIVO SOLICITADO
		std::string resourcePath = buildResourcePath(request, *locationRequest, server);
		if (!fileExistsAndReadable(resourcePath))
		{
			// Si no existe, intenta enviar página de error personalizada o respuesta 404 genérica
			processResponse.setStatusCode(404);
			processResponse.setBody("404 Not Found");
			_responsesToSend[socket.getSocketFd()] = processResponse;
			return ;
		}
		std::ifstream file(resourcePath.c_str(), std::ifstream::binary);
		if (file)
		{
			// Obtener el tamaño del archivo
			file.seekg(0, file.end);
			int length = file.tellg();
			file.seekg(0, file.beg);

			// Leer el contenido del archivo
			char* buffer = new char[length];
			file.read(buffer, length);

			if (file)
			{
				// Si se leyó con éxito, construir la respuesta
				processResponse.setStatusCode(200);
				processResponse.addHeader("Content-Type:", getMimeType(resourcePath));
				processResponse.setBody(std::string(buffer, length));
				if (request.getBody().size() > locationRequest->getMaxBodySize())
				{
					processResponse.setStatusCode(413);
					processResponse.setBody("413 Payload Too Large");
					_responsesToSend[socket.getSocketFd()] = processResponse;
					return;
				}
			}
			else
			{
				// Error al leer el archivo
				processResponse.setStatusCode(500);
				processResponse.setBody("500 Internal Server Error");
			}

			delete[] buffer;
			file.close();
		}
		else
		{
			// Archivo no se pudo abrir
			processResponse.setStatusCode(500);
			processResponse.setBody("500 Internal Server Error");
		}
		_responsesToSend[socket.getSocketFd()] = processResponse;
		return ;
	}
	else if (request.getMethod() == "POST")
	{
		
	}
	else
	{
		processResponse.setStatusCode(405);
		_responsesToSend[socket.getSocketFd()] = processResponse;
    	return ;
	}
}
std::string Server::buildResourcePath(HttpRequest& request,
	const Location& location, VirtualServers& server)
{
	// Extraer la URI de la solicitud
	std::string requestURI = request.getUri();

	// Eliminar cualquier parámetro de consulta de la URI
	size_t queryPos = requestURI.find('?');
	if (queryPos != std::string::npos)
		requestURI = requestURI.substr(0, queryPos);

	std::string basePath;
	if (!location.getRootLocation().empty())
		basePath = location.getRootLocation(); // Usar root de la location si está definido
	else
		basePath = server.getRoot(); // Usar root del servidor si no hay root en la location

	// Ajustar la ruta del recurso para manejo de directorios
	std::string resourcePath =
		adjustPathForDirectory(requestURI, basePath, location, server);
	if (!location.getAlias().empty())
		resourcePath.replace(0, location.getPath().length(), location.getAlias());
	return resourcePath;
}

std::string Server::adjustPathForDirectory(const std::string& requestURI, const std::string& basePath,
										const Location& location, VirtualServers& server)
{
	std::string fullPath = basePath + requestURI;
	std::string indexFile = location.getIndexLocation().empty() ? server.getIndex() : location.getIndexLocation();

	// Comprobar si la ruta completa apunta a un directorio
	if (isDirectory(fullPath))
	{
		// Construir la ruta al archivo índice dentro del directorio
		std::string indexPath = fullPath + (fullPath.back() == '/' ? "" : "/")
			+ indexFile;
		// Verificar si el archivo índice existe y es legible
		if (fileExistsAndReadable(indexPath))
			return indexPath;
	}
	else if (fileExistsAndReadable(fullPath))
		return fullPath;

	// Si ninguna de las anteriores, intentar como si fullPath fuera directamente el archivo solicitado
	// Esto es útil en caso de que fullPath ya incluya el archivo índice en la URI
	if (fileExistsAndReadable(fullPath))
		return fullPath;

	// Si ninguna ruta es válida, devuelve la ruta original (el manejo del error se realizará más adelante)
	return requestURI;
}

void Server::processReturnDirective(const Location& locationRequest,
	HttpResponse& processResponse)
{
	std::istringstream returnStream(locationRequest.getReturn());
	int statusCode;
	std::string urlOrText;

	// Extraer el código de estado y la URL o texto opcional
	returnStream >> statusCode;
	std::getline(returnStream, urlOrText);
	if (!urlOrText.empty() && urlOrText.front() == ' ')
		urlOrText.erase(0, 1); // Elimina el espacio inicial si existe

	// Configurar la respuesta basada en el código de estado
	processResponse.setStatusCode(statusCode);

	// Si es un código de redirección, añadir la URL a la cabecera 'Location'
	if (statusCode == 301 || statusCode == 302)
	{
		processResponse.addHeader("Location", urlOrText);
		processResponse.setBody(""); // El cuerpo de una respuesta de redirección suele estar vacío
	}
	else
		processResponse.setBody(urlOrText);
}
bool isDirectory(const std::string& path)
{
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return false;
	return S_ISDIR(statbuf.st_mode);
}

bool fileExistsAndReadable(const std::string& filePath)
{
	std::ifstream file(filePath.c_str());
	bool existsAndReadable = file.good();
	file.close();
	return existsAndReadable;
}

std::map<std::string, std::string> mimeTypes =
{
	{".html", "text/html"},
	{".css", "text/css"},
	{".js", "application/javascript"},
};

std::string getMimeType(const std::string& filePath)
{
	size_t dotPos = filePath.rfind('.');
	if (dotPos != std::string::npos)
	{
		std::string ext = filePath.substr(dotPos);
		if (mimeTypes.count(ext))
			return mimeTypes[ext];
	}
	return "text/plain"; // Tipo MIME por defecto si no se reconoce la extensión
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
