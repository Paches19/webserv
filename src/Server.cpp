/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:38:27 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/05 12:06:05 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

//******************************************************************************
// Constructores, destructor y operador de asignación
//******************************************************************************
Server::Server() { }
Server::Server(const Server& other)
{
	_serverSockets = other._serverSockets;
	_clientSockets = other._clientSockets;
	_connectionManager = other._connectionManager;
	_pollFds = other._pollFds;
	_responsesToSend = other._responsesToSend;
}
Server& Server::operator=(const Server& other)
{
	if (this != &other)
	{
		_serverSockets = other._serverSockets;
		_clientSockets = other._clientSockets;
		_connectionManager = other._connectionManager;
		_pollFds = other._pollFds;
		_responsesToSend = other._responsesToSend;
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

//******************************************************************************
// Getters
//******************************************************************************
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
	std::cout << "The server selected is : " << firstCandidate << std::endl;
	return servers[firstCandidate];
}

std::string Server::getMimeType(const std::string& filePath)
{
	size_t dotPos = filePath.rfind('.');
	std::map<std::string, std::string> mimeTypes;

	mimeTypes[".html"] = "text/html";
	mimeTypes[".css"]  = "text/css";
	mimeTypes[".js"]   = "application/javascript";
	mimeTypes[".jpg"]  = "image/jpeg";
	mimeTypes[".jpeg"] = "image/jpeg";
	mimeTypes[".png"]  = "image/png";
	mimeTypes[".gif"]  = "image/gif";
	mimeTypes[".svg"]  = "image/svg+xml";
	mimeTypes[".ico"]  = "image/x-icon";
	mimeTypes[".txt"]  = "text/plain";
	mimeTypes[".pdf"]  = "application/pdf";
	mimeTypes[".zip"]  = "application/zip";
	mimeTypes[".tar"]  = "application/x-tar";
	mimeTypes[".gz"]   = "application/gzip";
	mimeTypes[".mp3"]  = "audio/mpeg";
	mimeTypes[".mp4"]  = "video/mp4";
	mimeTypes[".avi"]  = "video/x-msvideo";
	mimeTypes[".mpeg"] = "video/mpeg";
	mimeTypes[".webm"] = "video/webm";
	mimeTypes[".json"] = "application/json";
	mimeTypes[".xml"]  = "application/xml";
	mimeTypes[".csv"]  = "text/csv";
	mimeTypes[".doc"]  = "application/msword";

	if (dotPos != std::string::npos)
	{
		std::string ext = filePath.substr(dotPos);
		if (mimeTypes.count(ext))
			return mimeTypes[ext];
	}
	return "text/plain"; // Tipo MIME por defecto si no se reconoce la extensión
}

//******************************************************************************
// Métodos de la clase
//******************************************************************************
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
					if (requestReceive.getIsValidRequest() && requestReceive.getIsCompleteRequest())
					{
						bestServer = getBestServer(requestReceive, i, servers);
						processRequest(requestReceive, bestServer, *dataSocket);				
					}
					else if (!requestReceive.getIsValidRequest())
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
						_connectionManager.writeData(*(_clientSockets[j]), bestServer, _responsesToSend[j]);
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
						_connectionManager.removeConnection(*(_clientSockets[j]), i, _pollFds, _clientSockets);
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
	std::cout << "    Searching for URL: " << request.getURL() << std::endl;
	
	std::string frontpage = server.getRoot();
	if (request.getURL() != "/")
		frontpage += request.getURL();
	if (server.getIndex()[0] != '/')
		frontpage += "/";
	frontpage += server.getIndex();
	std::cout << "    frontpage = " << frontpage << std::endl;
	
	std::vector<Location> locations = server.getLocations();
	const Location*	locationRequest = NULL;
	
	if (!locations.empty())
		locationRequest = locations[0].selectLocation(request.getURL(), locations);
	
	if (locationRequest == NULL)
	{
		std::cout << "    Location not found" << std::endl;
		processResponse.setStatusCode(404);

		std::string errorPage = server.getRoot();
		if (server.getErrorPage(404)[0] != '/')
			errorPage += "/";
		errorPage += server.getErrorPage(404);
		processResponse.setBody(ConfigFile::readFile(errorPage));
		_responsesToSend[socket.getSocketFd()] = processResponse;
		return ;
	}
	std::cout << "    Location found: " << locationRequest->getPath() << std::endl;
	std::string resourcePath = buildResourcePath(request, *locationRequest, server);
	std::cout << "    Resource path: " << resourcePath << std::endl;

	if (request.getMethod() == "GET")
	{
		//CONSTRUIMOS RUTA DEL ARCHIVO SOLICITADO
		if (!ConfigFile::fileExistsAndReadable(resourcePath))
		{
			// Si no existe, intenta enviar página de error personalizada o respuesta 404 genérica
			processResponse.setStatusCode(404);
			std::string errorPage = server.getRoot();
			if (locationRequest->getErrorPage(404)[0] != '/')
				errorPage += "/";
			errorPage += locationRequest->getErrorPage(404);
			processResponse.setBody(ConfigFile::readFile(errorPage));
			_responsesToSend[socket.getSocketFd()] = processResponse;
			return ;
		}
		std::cout << "    File exists and is readable" << std::endl;
		std::string buffer = ConfigFile::readFile(resourcePath);
		if (buffer.empty())
		{
			//Error del archivo:  vacío o no se pudo abrir
			processResponse.setStatusCode(500);
			processResponse.setBody("500 Internal Server Error");
			//processResponse.setBody(ConfigFile::readFile(server.getRoot() + "/" + locationRequest->getErrorPage(500)));
			return;
		}

		std::cout << "   body " << buffer << std::endl;
		if (buffer.length() > locationRequest->getMaxBodySize())
		{
			// Si el archivo es demasiado grande, enviar respuesta 413
			processResponse.setStatusCode(413);
			processResponse.setBody("413 Payload Too Large");
			//processResponse.setBody(ConfigFile::readFile(server.getRoot() + "/" + locationRequest->getErrorPage(413)));
			return;
		}
		// Si se leyó con éxito, construir la respuesta
		processResponse.setStatusCode(200);
		processResponse.setHeader("Content-Type:", getMimeType(resourcePath));
		processResponse.setBody(std::string(buffer, buffer.length()));
		_responsesToSend[socket.getSocketFd()] = processResponse;
		
	}
	else if (request.getMethod() == "POST")
	{
		processResponse.setStatusCode(200);
		processResponse.setHeader("Content-Type:", getMimeType(resourcePath));
		processResponse.setBody("");
		_responsesToSend[socket.getSocketFd()] = processResponse;
	}
	else if (request.getMethod() == "DELETE")
	{
		processResponse.setStatusCode(200);
		processResponse.setHeader("Content-Type:", getMimeType(resourcePath));
		processResponse.setBody("DELETE process");
		_responsesToSend[socket.getSocketFd()] = processResponse;
	}
	else
	{
		processResponse.setStatusCode(555);
		processResponse.setBody("XXX Request not supported");
	}
}

std::string Server::buildResourcePath(HttpRequest& request,
	const Location& location, VirtualServers& server)
{
	// Extraer la URL de la solicitud
	std::string requestURL = request.getURL();

	// Eliminar cualquier parámetro de consulta de la URL
	size_t queryPos = requestURL.find('?');
	if (queryPos != std::string::npos)
		requestURL = requestURL.substr(0, queryPos);

	std::string basePath = location.getRootLocation().empty() ? server.getRoot() : location.getRootLocation();

	// Ajustar la ruta del recurso para manejo de directorios
	std::string resourcePath =
		adjustPathForDirectory(requestURL, basePath, location, server);
	if (!location.getAlias().empty())
		resourcePath.replace(0, location.getPath().length(), location.getAlias());
	return resourcePath;
}

std::string Server::adjustPathForDirectory(const std::string& requestURL, const std::string& basePath,
										const Location& location, VirtualServers& server)
{
	std::string fullPath = basePath;
	if (requestURL != "/")
		fullPath += requestURL;
	std::cout << "    fullPath: " << fullPath << std::endl;

	std::string indexFile = location.getIndexLocation().empty() ? server.getIndex() : location.getIndexLocation();
	// Comprobar si la ruta completa apunta a un directorio
	if (ConfigFile::isDirectory(fullPath))
	{
		std::string indexPath = fullPath;
		if (indexFile[0] != '/')
			 indexPath +=  "/";
		indexPath += indexFile;
		// Construir la ruta al archivo índice dentro del directorio
		// Verificar si el archivo índice existe y es legible
		if (ConfigFile::fileExistsAndReadable(indexPath))
			return indexPath;
	}
	else if (ConfigFile::fileExistsAndReadable(fullPath))
		return fullPath;

	// Si ninguna de las anteriores, intentar como si fullPath fuera directamente el archivo solicitado
	// Esto es útil en caso de que fullPath ya incluya el archivo índice en la URL
	//if (ConfigFile::fileExistsAndReadable(fullPath))
	//	return fullPath;

	// Si ninguna ruta es válida, devuelve la ruta original (el manejo del error se realizará más adelante)
	return requestURL;
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
	if (!urlOrText.empty() && urlOrText[0] == ' ')
		urlOrText.erase(0, 1); // Elimina el espacio inicial si existe

	// Configurar la respuesta basada en el código de estado
	processResponse.setStatusCode(statusCode);

	// Si es un código de redirección, añadir la URL a la cabecera 'Location'
	if (statusCode == 301 || statusCode == 302)
	{
		processResponse.setHeader("Location", urlOrText);
		processResponse.setBody(""); // El cuerpo de una respuesta de redirección suele estar vacío
	}
	else
		processResponse.setBody(urlOrText);
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
