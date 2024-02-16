/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:38:27 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/16 12:28:17 by adpachec         ###   ########.fr       */
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
	std::cout << "\nInitializing server..." << std::endl;
	std::cout << "    Num. of servers: " << servers.size() << std::endl;
	_serverSockets.reserve(servers.size());

	// Crear sockets
	for (size_t i = 0; i < servers.size(); ++i)
	{
		if (checkOpenPorts(_serverSockets, servers[i]))
		{
			Socket* newSocket = new Socket();
			if (newSocket->open((int) servers[i].getPort(), servers[i].getIpAddress()) == false)
				throw ErrorException("Error opening the socket");
			_serverSockets.push_back(newSocket);
			struct pollfd serverPollFd;

			serverPollFd.fd = _serverSockets.back()->getSocketFd();
			serverPollFd.events = POLLIN; // Establecer para leer
			this->_pollFds.push_back(serverPollFd);
			std::cout << "    Listening on port:  " <<
				servers[i].getPort() << std::endl;
		}
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
// Métodos de la clase
//******************************************************************************
void Server::run(std::vector<VirtualServers> servers)
{
	std::cout << "\nServer running..." << std::endl;

	while (true)
	{
		fd_set readfds;
		for (size_t i = 0; i < _pollFds.size(); ++i)
			FD_SET(_pollFds[i].fd, &readfds);

		// Llamar a poll con la lista de file descriptors y un tiempo de espera
		int ret = poll(&_pollFds[0], _pollFds.size(), -1); // -1 para tiempo de espera indefinido
		if (ret < 0)
		{
			std::cerr << "    Poll error !" << std::endl;
			createErrorPage(500, servers[0], _serverSockets[0]);
			break;
		}

		// Revisar si hay nuevas conexiones en el socket del servidor
		for (size_t i = 0; i < _pollFds.size(); ++i)
		{
			HttpRequest requestReceive;
			VirtualServers bestServer;

			if (ret >= 0 && _pollFds[i].revents & POLLIN)
			{
				// std::cout << "\nPOLLIN i: " << i << std::endl;
				Socket* dataSocket = handleNewConnection(i);
				if (dataSocket && dataSocket->getSocketFd() != -1 &&
					_pollFds[i].fd == dataSocket->getSocketFd())
				{
					requestReceive = _connectionManager.readData(*dataSocket, i, _pollFds, _clientSockets);
					if (requestReceive.getIsValidRequest() && requestReceive.getIsCompleteRequest())
					{
						bestServer = getBestServer(requestReceive, i, servers, _clientSockets, _pollFds);
						std::cout << "Server: " << bestServer.getServerName() << std::endl;
						processRequest(requestReceive, bestServer, dataSocket);
					}
					else if (!requestReceive.getIsValidRequest())
					{
						if (_pollFds.size() > i - 1 && i > 0)
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
						_connectionManager.writeData(*(_clientSockets[j]), i, _responsesToSend[_pollFds[i].fd],
							_pollFds);
						break ;
					}
				}
			}
			else if (_pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				// Manejar desconexiones o errores
				std::cout << "    Connection closed or error in socket FD: " << _pollFds[i].fd << std::endl;
				int currentFd = _pollFds[i].fd;
				for (size_t j = 0; j < _clientSockets.size(); ++j)
				{
					if (_clientSockets[j]->getSocketFd() == currentFd)
					{
						std::cout << "Client socket deleted: " << _clientSockets[j]->getSocketFd() << std::endl;
						_connectionManager.removeConnection(*(_clientSockets[j]), i, _pollFds, _clientSockets);
						--i;
						break ;
					}
				}
			}
		}
	}
}


void Server::processRequest(HttpRequest request, VirtualServers server, Socket* socket)
{
	HttpResponse processResponse;

	// Configurar la respuesta
	std::cout << "\nProcessing REQUEST... " << std::endl;
	std::cout << "    Method: " << request.getMethod() << std::endl;
	std::cout << "    Requested URL: " << request.getURL() << std::endl;
	
	if (server.getPort() == 0)
	{
		std::cout << "    Server not found" << std::endl;
		createErrorPage(_errorCode, server, socket);
		return ;
	}
	std::vector<Location> locations = server.getLocations();
	const Location*	locationRequest = NULL;
	
	if (!locations.empty())
		locationRequest = locations[0].selectLocation(request.getURL(), locations);

	if (locationRequest == NULL)
	{
		std::cout << "    Location not found" << std::endl;
		createErrorPage(404,  server, socket);
		return ;
	}
	std::cout << "    Location found: " << locationRequest->getPath() << std::endl;
	if (locationRequest->getReturn()[0] != "")
	{
		processReturnDirective(*locationRequest, processResponse);
		_responsesToSend[socket->getSocketFd()] = processResponse;
		return ;
	}
	std::string resourcePath = buildResourcePath(request, *locationRequest, server);
	std::cout << "    Resource path: " << resourcePath << std::endl;

	//****************************GET Method****************************
	if (request.getMethod() == "GET")
	{
		if (!locationRequest->getMethods()[GET_METHOD])
		{
			createErrorPage(405, server, socket);
			return ;
		}
		processGet(resourcePath, locationRequest, socket, server, request);
	}
	//****************************POST Method****************************
	else if (request.getMethod() == "POST")
	{
		// Verificar si el método POST está permitido
		if (!locationRequest->getMethods()[POST_METHOD])
		{
			createErrorPage(405, server, socket);
			return ;
		}
		// Verificar si el Content-Length excede el máximo permitido
		processPost(request, server, socket, locationRequest);
	}
	//****************************DELETE Method****************************
	else if (request.getMethod() == "DELETE")
	{
		// Verificar si el método DELETE está permitido
		if (!locationRequest->getMethods()[DELETE_METHOD])
		{
			createErrorPage(405, server, socket);
			return ;
		}
		processDelete(resourcePath, server, socket);
	}
	//****************************Unknown Method****************************
	else
	{
		// Método no soportado
		processResponse.setStatusCode(555);
		processResponse.setHeader("Content-Type", "text/plain");
		processResponse.setBody("Request Method not supported");
		_responsesToSend[socket->getSocketFd()] = processResponse;
	}
}

void Server::processGet(std::string resourcePath, const Location* locationRequest,
	Socket* socket, VirtualServers server, HttpRequest request)
{
	HttpResponse processResponse;
	resourcePath = checkGetPath(resourcePath, locationRequest, socket, server);
	if (resourcePath.empty())
		return ;
	std::cout << "devuelvo: " << resourcePath << std::endl;
	// Check if the requested resource is a CGI script
	if (isCGIScript(resourcePath))
	{
	// Perform CGI processing
		executeCGIScript(resourcePath, request, processResponse, server, socket);
		return;
	}
	std::string buffer = ConfigFile::readFile(resourcePath);
	if (buffer.empty())
	{
		//Error si el archivo está vacío o no se pudo abrir
		createErrorPage(500, server, socket);
		return;
	}

	if (buffer.size() > locationRequest->getMaxBodySize())
	{
		// Error si el archivo es demasiado grande
		createErrorPage(413,  server, socket);
		return;
	}

	// Si se leyó con éxito, construir la respuesta
	processResponse.setStatusCode(200);
	processResponse.setHeader("Content-Type", getMimeType(resourcePath));
	processResponse.setBody(buffer);
	_responsesToSend[socket->getSocketFd()] = processResponse;
}

void Server::processPost(HttpRequest request, VirtualServers server, Socket* socket,
	const Location* locationRequest)
{
	HttpResponse processResponse;
	
	std::string contentLengthHeader = request.getHeader("Content-Length");
	unsigned long contentLength;
	if (contentLengthHeader.empty())
		contentLength = 0;
	else
		contentLength = std::strtoul(contentLengthHeader.c_str(), NULL, 10);
	if (contentLength > server.getClientMaxBodySize())
	{
		createErrorPage(413, server, socket);
		return;
	}

	// Verificar si el tipo de contenido es soportado (ejemplo: no se soporta multipart/form-data o chunked)
	std::string contentTypeHeader = request.getHeader("Content-Type");
	if (contentTypeHeader.find("multipart/form-data") != std::string::npos ||
		contentTypeHeader.find("chunked") != std::string::npos)
	{
		createErrorPage(501, server, socket);
		return ;
	}

	// Determinar la ruta absoluta donde se guardará el contenido de la solicitud POST
	// Error si la ruta es inválida o no se puede escribir
	std::string resourcePath = buildResourcePathForPost(request, *locationRequest, server);
	std::cout << "ResourcePath: " << resourcePath << std::endl;
	if (resourcePath.empty() || !isValidPath(locationRequest->getRootLocation().empty() ? server.getRoot()
		: locationRequest->getRootLocation(), resourcePath))
	{
		createErrorPage(400, server, socket);
		return ;
	}
	// Guardar el cuerpo de la solicitud en el archivo especificado por la ruta
	// Error si no se puede abrir el archivo
	std::string fullResourcePath = getFilename(request, resourcePath);
	if (!postFile(fullResourcePath, request, server, socket))
		return ;

	// Guardar el cuerpo de la solicitud en el archivo especificado por la ruta	
	processResponse.setStatusCode(200);
	processResponse.setHeader("Content-Type", "text/plain");
	processResponse.setBody("Content uploaded successfully.");
	_responsesToSend[socket->getSocketFd()] = processResponse;
}

void Server::processDelete(std::string resourcePath, VirtualServers server, Socket* socket)
{
	HttpResponse processResponse;
	
	if (!ConfigFile::fileExistsAndReadable(resourcePath))
	{
		createErrorPage(404, server, socket);
		return;
	}
	// Eliminar el recurso
	if (remove(resourcePath.c_str()) != 0)
	{
		createErrorPage(500, server, socket);
		return;
	}
	// Construir la respuesta
	processResponse.setStatusCode(204);
	processResponse.setBody("");
	_responsesToSend[socket->getSocketFd()] = processResponse;
}

void Server::createErrorPage(short errorCode, VirtualServers &server, Socket* socket)
{
	HttpResponse response;
	
	response.setStatusCode(errorCode);
	
	std::string errorPage1 = server.getRoot();
	std::string errorPage2 = server.getErrorPage(errorCode);

	if (errorPage2 == "")
		response.setBody(createBodyErrorPage(errorCode));
	else if (errorPage2[0] != '/')
		errorPage2 = "/" + errorPage2;
	errorPage2 = errorPage1 + errorPage2;
	if (ConfigFile::fileExistsAndReadable(errorPage2))
	{
		std::string bodyFromFile = ConfigFile::readFile(errorPage2);
		response.setBody(bodyFromFile);
	}
	else
		response.setBody(createBodyErrorPage(errorCode));
	_responsesToSend[socket->getSocketFd()] = response;
}

void Server::executeCGIScript(std::string& scriptPath, HttpRequest& request, 
	HttpResponse& response, VirtualServers& server, Socket* socket)
{
    pid_t pid = fork();

    if (pid == -1) // Error handling if fork fails
        createErrorPage(500, server, socket);

    else if (pid == 0)  // Child process
    {
        // Redirect stdout to a pipe
        int pipefd[2];
        if (pipe(pipefd) == -1)
		{
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pid_t scriptPid = fork();// Fork a child process to execute the CGI script
        if (scriptPid == -1)
		{
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (scriptPid == 0) // Child process (script execution)
		{
        	// Close read end of the pipe
            close(pipefd[0]);

            // Redirect stdout to the write end of the pipe
            dup2(pipefd[1], STDOUT_FILENO);

            // Set up arguments for execve
            const char* scriptName = scriptPath.c_str();
            char* const argv[] = { const_cast<char*>(scriptName), const_cast<char*>(request.getURL().c_str()), NULL };
			// Prepare environment variables
            std::vector<char*> envp;
            // Add your custom environment variables
			//envp.push_back(strdup("CUSTOM_VARIABLE=value"));
 			envp.push_back(NULL); // Terminate the envp array
			/*Some CGI environment variables for Chrome browser:
				HTTP_ACCEPT: Specifies the media types that the client can process, such as text/html or image/png.
				HTTP_ACCEPT_LANGUAGE: Indicates the natural languages that the client can understand.
				HTTP_USER_AGENT: Identifies the user agent (browser) making the request, including information about the browser type and version, as well as the operating system.
				HTTP_REFERER: Contains the URL of the page that referred the user to the current page.
				HTTP_COOKIE: Holds any cookies that have been sent by the server to the client.
				HTTP_HOST: Provides the domain name of the server.
				REMOTE_ADDR: Represents the IP address of the client.
				REMOTE_PORT: Specifies the port number on the client.
				REQUEST_METHOD: Indicates the HTTP request method, such as GET or POST.
				QUERY_STRING: Holds the query string portion of the URL for a GET request.
				CONTENT_LENGTH: Specifies the size of the message body for requests with a body, such as POST requests.
				CONTENT_TYPE: Describes the type of data in the body of the request, particularly relevant for POST requests.
				PATH_INFO: Contains the path information that follows the actual script name in the URL.
			*/		
			char* const* envpArray = envp.data(); // Convert to char* array

            execve(scriptName, argv, envpArray);// Execute the CGI script

            // If execve() is successful, this code won't be reached
            perror("execve");
            exit(EXIT_FAILURE);
        }
        else // Parent process (script execution)
		{
			close(pipefd[1]);// Close write end of the pipe

            // Read from the read end of the pipe and store in response body
            char buffer[4096];
            ssize_t bytesRead;
			std::string body;

            while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
				body.append(buffer, bytesRead);
            response.setBody(body);

            close(pipefd[0]);// Close the read end of the pipe

            // Wait for the CGI script to complete
            int scriptStatus;
            waitpid(scriptPid, &scriptStatus, 0);

            // Handle the child process completion status if needed

            exit(EXIT_SUCCESS);// Exit the child process
        }
    }
    else  // Parent process
    {
        // Wait for the child to complete
        int status;
        waitpid(pid, &status, 0);

        // Set appropriate CGI-related response headers
        response.setStatusCode(200);
        response.setHeader("Content-Type", "text/html");
        _responsesToSend[socket->getSocketFd()] = response;
    }
}

void Server::processReturnDirective(const Location& locationRequest,
	HttpResponse& processResponse)
{
	std::vector<std::string> ret = locationRequest.getReturn();
	int statusCode = Location::ft_stoi(ret[0]);
	std::string urlOrText = locationRequest.getReturn()[1];

	if (urlOrText[0] == '/')
	{
		// Si la cadena comienza con '/', es una redirección interna
		// La URL se construye a partir de la raíz del servidor
		// Si es un código de redirección, añadir la URL a la cabecera 'Location'
		processResponse.setHeader("Location", urlOrText);
		if (statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 307)
			processResponse.setBody("");
		else
			processResponse.setBody(bodyReturn(locationRequest.getPath(), urlOrText, statusCode));
	}
	else
	{	// si no, se envía el texto de la redirección
		processResponse.setHeader("Location", locationRequest.getPath());
		if (statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 307)
			processResponse.setBody("");
		else
			processResponse.setBody(bodyReturn(urlOrText, "", statusCode));
	// Configurar la respuesta basada en el código de estado
	}
	processResponse.setStatusCode(statusCode);
}

Socket* Server::handleNewConnection(int i)
{
	Socket* existingSocket;
	if (i < (int) _serverSockets.size())
	{
		Socket* newSocket = new Socket();
		if (_serverSockets[i]->accept(*newSocket, _serverSockets[i]->getListenPort()))
		{
			std::cout << "Accept " << i << " server port: " << _serverSockets[i]->getListenPort() << std::endl;
			std::vector<Socket*>::iterator it;
			for (it = _clientSockets.begin(); it != _clientSockets.end(); ++it)
			{
				existingSocket = *it;
				if (areAddressesEqual(newSocket->getSocketAddr(), existingSocket->getSocketAddr()))
				{
					//std::cout << "    Cliente existente" << std::endl;
					delete newSocket;
					return existingSocket;
				}
			}
			//std::cout << "    Nueva conexión";
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
			std::cerr << "    Error accepting new connection" << std::endl;
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

bool Server::postFile(std::string resourcePath, HttpRequest request, VirtualServers server, 
	Socket* socket)
{
	HttpResponse processResponse;

	if (resourcePath[0] == '.')
		resourcePath = resourcePath.substr(2);
	std::ofstream outputFile(resourcePath.c_str(), std::ios::out | std::ios::binary);
	if (!outputFile.is_open())
	{
		createErrorPage(500, server, socket);
		return false;
	}
	outputFile.write(request.getBody().c_str(), request.getBody().size());
	outputFile.close();
	return true;
}

std::string Server::checkGetPath(std::string resourcePath, const Location* locationRequest,
		Socket* socket, VirtualServers server)
{
	HttpResponse processResponse;
	if (ConfigFile::checkPath(resourcePath) == IS_DIR)
	{
		// std::cout << " Es directorio " << std::endl;
		
		if (locationRequest->getAutoindex())
		{
			// Autoindex activado: generar y enviar página de índice
			std::string directoryIndexHTML = generateDirectoryIndex(resourcePath);
			processResponse.setStatusCode(200);
			processResponse.setHeader("Content-Type:", "text/html");
			processResponse.setBody(directoryIndexHTML);
			_responsesToSend[socket->getSocketFd()] = processResponse;
			return "";
		}
		else
		{
			// Autoindex desactivado: buscar archivo index por defecto
			std::string indexPath = resourcePath;
			if (ConfigFile::fileExistsAndReadable(indexPath))
			{
				// Enviar archivo index
				std::string buffer = ConfigFile::readFile(indexPath);
				
				processResponse.setStatusCode(200);
				processResponse.setHeader("Content-Type:", getMimeType(indexPath));
				processResponse.setBody(buffer);
				_responsesToSend[socket->getSocketFd()] = processResponse;
				return "";
			}
			else
			{
				// Directorio sin archivo index y autoindex desactivado
				createErrorPage(403, server, socket);
				return "";
			}
		}
	}
	else if (!ConfigFile::fileExistsAndReadable(resourcePath))
	{
		// Si no existe, intenta enviar página de error personalizada o respuesta 404 genérica
		createErrorPage(404, server, socket);
		return "";
	}
	//std::cout << "    File exists and is readable" << std::endl;
	return resourcePath;
}
