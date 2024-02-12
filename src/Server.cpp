/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:38:27 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/05 18:12:36 by adpachec         ###   ########.fr       */
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
		Socket* newSocket = new Socket();
		if (newSocket->open((int) servers[i].getPort(), servers[i].getIpAddress()) == false)
			throw ErrorException("Error opening the socket");
		_serverSockets.push_back(newSocket);
		
		struct pollfd serverPollFd;

		serverPollFd.fd = _serverSockets[i]->getSocketFd();
		serverPollFd.events = POLLIN; // Establecer para leer
		this->_pollFds.push_back(serverPollFd);
		std::cout << "    Listening on port:  " <<
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
	if (j == _clientSockets.size()) //No encuentra cliente
	{
		_errorCode = 500;
		VirtualServers aServer;
		return (aServer);
	}
	
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
	
	// Si no encuentra servers --> ERROR 404
	if (nbServer == 0)
	{
		_errorCode = 404;
		VirtualServers aServer;
		return aServer;
	}
				
	// Si hay varios candidatos......
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
	//std::cout << "Selected Server: " << firstCandidate << std::endl;
	return servers[firstCandidate];
}

std::string Server::getMimeType(const std::string& filePath)
{
	size_t dotPos = filePath.rfind('.');
	std::map<std::string, std::string> mimeTypes;

	mimeTypes[".html"] = "text/html";
	mimeTypes[".css"]  = "text/css";
	mimeTypes[".txt"]  = "text/plain";
	mimeTypes[".csv"]  = "text/csv";
	mimeTypes[".htm"]  = "text/html";
	
	mimeTypes[".jpg"]  = "image/jpeg";
	mimeTypes[".jpeg"] = "image/jpeg";
	mimeTypes[".png"]  = "image/png";
	mimeTypes[".gif"]  = "image/gif";
	mimeTypes[".svg"]  = "image/svg+xml";
	mimeTypes[".ico"]  = "image/x-icon";

	mimeTypes[".pdf"]  = "application/pdf";
	mimeTypes[".zip"]  = "application/zip";
	mimeTypes[".tar"]  = "application/x-tar";
	mimeTypes[".gz"]   = "application/gzip";
	mimeTypes[".js"]   = "application/javascript";
	mimeTypes[".json"] = "application/json";
	mimeTypes[".xml"]  = "application/xml";
	mimeTypes[".doc"]  = "application/msword";

	mimeTypes[".mp3"]  = "audio/mpeg";
	mimeTypes[".mp4"]  = "video/mp4";
	mimeTypes[".avi"]  = "video/x-msvideo";
	mimeTypes[".mpeg"] = "video/mpeg";
	mimeTypes[".webm"] = "video/webm";

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
			//createErrorPage(500, _responsesToSend[_pollFds[0].fd], servers[0], _serverSockets[0]);
			//break;
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
						bestServer = getBestServer(requestReceive, i, servers);
						processRequest(requestReceive, bestServer, dataSocket);
					}
					else if (!requestReceive.getIsValidRequest()) // Bad Request
					{
						if (_pollFds.size() > i - 1 && i > 0)
							--i;
					}
				}
			}
			else if ((_pollFds[i].revents & POLLOUT))
			{
				// std::cout << "\nPOLLOUT i: " << i << std::endl;
				for (size_t j = 0; j < _clientSockets.size(); ++j)
				{
					if (_clientSockets[j]->getSocketFd() == _pollFds[i].fd)
					{
						// std::cout << "\nENTRO POLLOUT por fd igual" << std::endl;
						// std::cout << "Response a enviar en fd: " << _pollFds[i].fd << std::endl;
						// std::cout << "dataResponseSent preWriteData: " << _connectionManager.connections[_pollFds[i].fd].responseSent << std::endl;
						_connectionManager.writeData(*(_clientSockets[j]), i, _responsesToSend[_pollFds[i].fd],
							_pollFds);
						//std::cout << "dataResponseSent postWriteData: " << _connectionManager.connections[_pollFds[i].fd].responseSent << std::endl;
						//std::cout << "Response a enviar en fd: " << _pollFds[i].fd << std::endl;
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
		} // Fin del bucle for (recorre todos los file descriptors que se están escuchando)
	} // Fin del bucle while (always true)
}


std::string Server::createBodyErrorPage(short &errorCode)
{
	HttpResponse msg;
	std::ostringstream errorCodeS;
	errorCodeS << errorCode;
	std::string errorPage = "<html>\n<head>\n<title>Error " + errorCodeS.str() + "</title>\n</head>\n";
		errorPage += "<body>\n<h1>Error " + errorCodeS.str() + "</h1>\n";
		errorPage += "<p>" + errorCodeS.str() + " " + msg.getStatusMessage(errorCode) + "</p>\n";
		errorPage += "</body>\n</html>";
	return errorPage;
}

void Server::createErrorPage(short errorCode, HttpResponse &response, VirtualServers &server, Socket* socket)
{
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

bool isValidPath(const std::string& basePath, const std::string& path)
{
	// Prevenir Path Traversal verificando la presencia de ".."
	if (path.find("..") != std::string::npos)
		return false;

	std::string fullPath = path;
	// Verificar si la ruta completa es un directorio permitido
	if (ConfigFile::checkPath(fullPath) != 2)
		return false; // No es un directorio o no es accesible

	// Asegurarse de que el path no salga del directorio base
	if (fullPath.find(basePath) != 0)
		return false; // El path resultante no está dentro del basePath

	return true; // La ruta es válida y está permitida
}
bool isCGIScript(const std::string& resourcePath)
{
    // Determine if the resource is a CGI script based on some criteria
    // For example, check the file extension or any other condition
    // Return true if it's a CGI script, false otherwise

    // Example: Check if the file extension is ".cgi"
    if (resourcePath.size() >= 4)
	{
		if ((resourcePath.substr(resourcePath.size() - 3) == ".py")  ||
			(resourcePath.substr(resourcePath.size() - 3) == ".sh"))
			{
				std::cout << "    CGI script detected" << std::endl;
				return true;
			}
	}
	return false;
}

void Server::executeCGIScript(std::string& scriptPath, HttpRequest& request, 
	HttpResponse& response, VirtualServers& server, Socket* socket)
{
    pid_t pid = fork();

    if (pid == -1) // Error handling if fork fails
        createErrorPage(500, response, server, socket);

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
		createErrorPage(_errorCode, processResponse, server, socket);
		return ;
	}
	std::vector<Location> locations = server.getLocations();
	const Location*	locationRequest = NULL;
	
	if (!locations.empty())
		locationRequest = locations[0].selectLocation(request.getURL(), locations);

	if (locationRequest == NULL)
	{
		std::cout << "    Location not found" << std::endl;
		createErrorPage(404, processResponse, server, socket);
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
			createErrorPage(405, processResponse, server, socket);
			return ;
		}
		resourcePath = checkGetPath(resourcePath, locationRequest, socket, server);
		if (resourcePath.empty())
			return ;
		
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
			createErrorPage(500, processResponse, server, socket);
			return;
		}

		if (buffer.size() > locationRequest->getMaxBodySize())
		{
			// Error si el archivo es demasiado grande
			createErrorPage(413, processResponse, server, socket);
			return;
		}

		// Si se leyó con éxito, construir la respuesta
		processResponse.setStatusCode(200);
		processResponse.setHeader("Content-Type", getMimeType(resourcePath));
		processResponse.setBody(buffer);
		_responsesToSend[socket->getSocketFd()] = processResponse;
	}

	//****************************POST Method****************************
	else if (request.getMethod() == "POST")
	{
		// Verificar si el método POST está permitido
		if (!locationRequest->getMethods()[POST_METHOD])
		{
			createErrorPage(405, processResponse, server, socket);
			return ;
		}
		// Verificar si el Content-Length excede el máximo permitido
		std::string contentLengthHeader = request.getHeader("Content-Length");
		unsigned long contentLength;
		if (contentLengthHeader.empty())
			contentLength = 0;
		else
			contentLength = std::strtoul(contentLengthHeader.c_str(), NULL, 10);
		if (contentLength > server.getClientMaxBodySize())
		{
			createErrorPage(413, processResponse, server, socket);
			return;
		}

		// Verificar si el tipo de contenido es soportado (ejemplo: no se soporta multipart/form-data o chunked)
		std::string contentTypeHeader = request.getHeader("Content-Type");
		if (contentTypeHeader.find("multipart/form-data") != std::string::npos ||
			contentTypeHeader.find("chunked") != std::string::npos)
		{
			createErrorPage(501, processResponse, server, socket);
			return ;
		}

		// Determinar la ruta absoluta donde se guardará el contenido de la solicitud POST
		// Error si la ruta es inválida o no se puede escribir
		std::string resourcePath = buildResourcePathForPost(request, *locationRequest, server);
		if (resourcePath.empty() || !isValidPath(locationRequest->getRootLocation().empty() ? server.getRoot()
			: locationRequest->getRootLocation(), resourcePath))
		{
			createErrorPage(400, processResponse, server, socket);
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
	//****************************DELETE Method****************************
	else if (request.getMethod() == "DELETE")
	{
		// Verificar si el método DELETE está permitido
		if (!locationRequest->getMethods()[DELETE_METHOD])
		{
			createErrorPage(405, processResponse, server, socket);
			return ;
		}
		// Verificar si el recurso existe y es legible
		if (!ConfigFile::fileExistsAndReadable(resourcePath))
		{
			createErrorPage(404, processResponse, server, socket);
			return;
		}
		// Eliminar el recurso
		if (remove(resourcePath.c_str()) != 0)
		{
			createErrorPage(500, processResponse, server, socket);
			return;
		}
		// Construir la respuesta
		processResponse.setStatusCode(204);
		processResponse.setBody("");
		_responsesToSend[socket->getSocketFd()] = processResponse;
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

bool Server::postFile(std::string resourcePath, HttpRequest request, VirtualServers server, 
	Socket* socket)
{
	HttpResponse processResponse;
	std::ofstream outputFile(resourcePath.c_str(), std::ios::out | std::ios::binary);
	if (!outputFile.is_open())
	{
		createErrorPage(500, processResponse, server, socket);
		return false;
	}
	outputFile.write(request.getBody().c_str(), request.getBody().size());
	outputFile.close();
	return true;
}

std::string Server::getFilename(HttpRequest request, std::string resourcePath)
{
	std::string filename;
	std::string contentDispositionHeader = request.getHeader("Content-Disposition");
	//std::cout << "contentDispositionHeader: " << contentDispositionHeader << std::endl;
	size_t filenamePos = contentDispositionHeader.find("filename=");

	if (filenamePos != std::string::npos)
	{
		// Extraer el nombre del archivo
		size_t filenameStart = filenamePos + strlen("filename=");
		// Buscar la primera comilla doble después del "filename="
		size_t quotePos = contentDispositionHeader.find("\"", filenameStart);
		if (quotePos != std::string::npos)
		{
			// La posición de inicio del nombre del archivo es después de la primera comilla doble
			size_t filenameStartPos = quotePos + 1;
			// Buscar la siguiente comilla doble para determinar el final del nombre del archivo
			size_t filenameEndPos = contentDispositionHeader.find("\"", filenameStartPos);
			if (filenameEndPos != std::string::npos)
			{
				// Extraer el nombre del archivo entre las comillas dobles
				filename = contentDispositionHeader.substr(filenameStartPos, filenameEndPos - filenameStartPos);
				//std::cout << "filename: " << filename << std::endl;
			}
		}
		else
		{
			size_t spacePos = contentDispositionHeader.find(" ", filenameStart);
			if (spacePos != std::string::npos)
			{
				// Extraer el nombre del archivo entre "filename=" y el primer espacio en blanco
				filename = contentDispositionHeader.substr(filenameStart, spacePos - filenameStart);
				//std::cout << "filename: " << filename << std::endl;
			}
			else
			{
				size_t filenameStart = filenamePos + strlen("filename=");
				size_t filenameEnd = contentDispositionHeader.length();
				// Extraer el nombre del archivo desde filenameStart hasta el final de la cadena
				filename = contentDispositionHeader.substr(filenameStart, filenameEnd - filenameStart);
				//std::cout << "filename: " << filename << std::endl;
			}
		}
	}
	else
	{
		size_t lastSlashPos = request.getURL().find_last_of('/');
		if (lastSlashPos != std::string::npos)
			filename = request.getURL().substr(lastSlashPos + 1);
	}
	if (!filename.empty())
		resourcePath += "/" + filename;
	if (resourcePath.size() >= 2 && resourcePath[0] == '/')
		resourcePath = &resourcePath[1];
	return resourcePath;

	// Concatenar el nombre del archivo al resourcePath
	//std::string resourcePath = resourcePath + "/" + filename;
	//if (resourcePath.size() >= 2)
	//	resourcePath = &resourcePath[1];
	//return resourcePath;
}

std::string Server::checkGetPath(std::string resourcePath, const Location* locationRequest,
		Socket* socket, VirtualServers server)
{
	HttpResponse processResponse;
	if (ConfigFile::checkPath(resourcePath) == IS_DIR)
	{
		//std::cout << " Es directorio " << std::endl;
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
				createErrorPage(403, processResponse, server, socket);
				return "";
			}
		}
	}
	else if (!ConfigFile::fileExistsAndReadable(resourcePath))
	{
		// Si no existe, intenta enviar página de error personalizada o respuesta 404 genérica
		createErrorPage(404, processResponse, server, socket);
		return "";
	}
	//std::cout << "    File exists and is readable" << std::endl;
	return resourcePath;
}

std::string Server::generateDirectoryIndex(const std::string& directoryPath)
{
	std::stringstream html;
	html << "<html>\n<head>\n<title>Index of " << directoryPath << "</title>\n</head>\n";
	html << "<body>\n<h1>Index of " << directoryPath << "</h1>\n";
	html << "<ul>\n";

	DIR* dir = opendir(directoryPath.c_str());
	if (dir != NULL)
	{
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL)
		{
			// Filtra "." y ".."
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			// Construye el enlace al archivo/directorio
			html << "<li><a href=\"" << directoryPath + "/"+entry->d_name << "\">" << entry->d_name << "</a></li>\n";
		}
		closedir(dir);
	}
	else
		html << "<p>Error opening directory.</p>\n";

	html << "</ul>\n</body>\n</html>";
	return html.str();
}

std::string Server::buildResourcePathForPost(HttpRequest& request,
	const Location& location, VirtualServers& server)
{
	std::string requestURL = request.getURL();

	// Eliminar parámetros de consulta
	size_t queryPos = requestURL.find('?');
	if (queryPos != std::string::npos)
		requestURL = requestURL.substr(0, queryPos);

	// Prevenir Path Transversal
	if (requestURL.find("..") != std::string::npos)
		return "";

	std::string basePath = location.getRootLocation().empty() ? server.getRoot() : location.getRootLocation();

	if (basePath != "/" && !basePath.empty() && basePath[basePath.length() - 1] == '/')
        basePath.erase(basePath.length() - 1);

	if (!requestURL.empty() && requestURL[0] != '/' && basePath != "/")
        requestURL = "/" + requestURL;
	
	std::string resourcePath = basePath + requestURL;

	// Prevenir la creación de archivos fuera del directorio raíz
	if (!request.startsWith(resourcePath, basePath))
		return "";

	return resourcePath;
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
	if (fullPath[0] != '.')
		fullPath = "." + fullPath;
	
	//std::cout << "    fullPath: " << fullPath << std::endl;
	std::string indexFile = location.getIndexLocation().empty() ? server.getIndex() : location.getIndexLocation();
	// Comprobar si la ruta completa apunta a un directorio
	if (ConfigFile::checkPath(fullPath) == IS_DIR)
	{
		std::string indexPath = fullPath;
		if (indexFile[0] != '/')
			 indexPath +=  "/";
		indexPath += indexFile;
		// Construir la ruta al archivo índice dentro del directorio
		// Verificar si el archivo índice existe y es legible
		if (ConfigFile::fileExistsAndReadable(indexPath))
			return indexPath;
		else
			return fullPath;
	}
	// Si ninguna de las anteriores, intentar como si fullPath fuera directamente el archivo solicitado
	// Esto es útil en caso de que fullPath ya incluya el archivo índice en la URL
	else if (ConfigFile::fileExistsAndReadable(fullPath))
		return fullPath;
	// Si ninguna ruta es válida, devuelve la ruta original (el manejo del error se realizará más adelante)
	return requestURL;
}

std::string bodyReturn(const std::string cad, const std::string& url, int statusCode)
{
	HttpResponse r;
	std::stringstream ss;
	ss << statusCode;

	std::string body = "<html>\n<head>\n<title>";
	body += ss.str() + " " +r.getStatusMessage(statusCode) + "</title>\n</head>\n";
	body += "<body>\n<h1>" + ss.str() + " " + r.getStatusMessage(statusCode) + "</h1>\n";
	if (url != "")
		body += "<p><h2><font color=\"green\">Redirecting to <a href=\"" + url + "\">" + url + "</a></font></h2></p>\n";
	else
		body += "<p><h2><font color=\"red\">" + cad + "</font></h2></p>\n";
	body += "</body>\n</html>";
	return body;
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
