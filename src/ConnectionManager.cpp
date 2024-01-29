/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:42:54 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/24 12:59:14 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionManager.hpp"

ConnectionManager::ConnectionManager() {}

ConnectionManager::~ConnectionManager() {}

void ConnectionManager::addConnection(Socket& socket)
{
	int socketFd = socket.getSocketFd();
	ConnectionData connData;

	connections.insert(std::make_pair(socketFd, connData));
	std::cout << "    Agregada con Socket FD: " << socketFd << std::endl;
}

void ConnectionManager::removeConnection(Socket& socket)
{
    int socketFd = socket.getSocketFd();
    std::map<int, ConnectionData>::iterator it = connections.find(socketFd);
    if (it != connections.end())
    {
        socket.close();
        connections.erase(it);
        std::cout << "\n****Conexión eliminada. Socket FD: " << socketFd << std::endl;
    }
    else
    {
        std::cout << "\n****Conexión no encontrada. Socket FD: " << socketFd << std::endl;
    }
}

bool ConnectionManager::readData(Socket& socket)
{
	ConnectionData data(connections[socket.getSocketFd()]);

	std::cout << "\n****Socket de Lectura: " << socket.getSocketFd() << std::endl;
	// Leer datos del socket
	int bytesRead = socket.receive(&data.readBuffer[0], data.readBuffer.size());
	std::cout << "    Bytes Read: " << bytesRead << std::endl;
	std::cout << "    data.readBuffer.empty: " << data.readBuffer.empty() << std::endl;

	if (bytesRead > 0)
	{
		std::cout << "    ReadBuffer size: " << data.readBuffer.size() << std::endl;
		data.accumulatedBytes += bytesRead; // Añadir a la cuenta de bytes acumulados

		if (isHttpRequestComplete(data.readBuffer, data.accumulatedBytes))
		{
			// Procesar la solicitud completa
			HttpRequest request(std::string(data.readBuffer.begin(), data.readBuffer.end()));

			std::cout << YELLOW << "\n****REQUEST recibida:" << std::endl;
			std::cout << "    Method: " << request.getMethod() << std::endl;
			std::cout << "    URL: " << request.getURL() << std::endl;
			std::cout << "    HTTP Version: " << request.getHttpVersion() << std::endl;
			std::cout << "    Headers :" << std::endl;
			std::map<std::string, std::string> headers = request.getHeaders();
			std::map<std::string, std::string>::const_iterator it;
			for (it = headers.begin(); it != headers.end(); ++it)
				std::cout << "    " << it->first << ": " << it->second << std::endl;
			std::cout << RESET << std::endl;
			if (request.isValidRequest())
				data.responseSent = false;
			else
			{
				std::cerr << "    Invalid request" << std::endl;
				return false;
			}
			
			data.readBuffer.clear();
			data.readBuffer.resize(1024);
			data.accumulatedBytes = 0;
			data.headerReceived = false;
			_request = request;
			connections[socket.getSocketFd()] = data;
		}
	}
	else if (bytesRead == 0) 
		return false;
	else 
	{
		data.readBuffer.clear();
		data.readBuffer.resize(1024);
		data.accumulatedBytes = 0;
		data.headerReceived = false;
		connections[socket.getSocketFd()] = data;
	}
	return true;
}

void ConnectionManager::writeData(Socket& socket, VirtualServers &server) 
{
	ConnectionData& data(connections[socket.getSocketFd()]);

	if (data.responseSent)
		return;
	
	std::cout << "\n****Request leida para preparar RESPUESTA" << std::endl;
	std::cout << "    Method: " << _request.getMethod() << std::endl;
	std::cout << "    URL: " << _request.getURL() << std::endl;
	std::cout << "    HTTP Version: " << _request.getHttpVersion() << std::endl;
	std::cout << "    Headers :" << std::endl;
	std::map<std::string, std::string> headers = _request.getHeaders();
	std::map<std::string, std::string>::const_iterator it0;
	for (it0 = headers.begin(); it0 != headers.end(); ++it0)
		std::cout << "    " << it0->first << ": " << it0->second << std::endl;
	std::cout << std::endl;
	
	ResponseBuilder responseBuilder;

	// Configurar la respuesta
	std::cout << "\n****METODO DETECTADO EN REQUEST: " << _request.getMethod() << std::endl;
	std::cout << "    Searching for URL: " << _request.getURL() << std::endl;
	std::string frontpage = server.getRoot() + server.getIndex();


	std::cout << "    Searching frontpage: " << frontpage << std::endl;
	std::vector<Location> locations = server.getLocations();
	std::vector<Location>::const_iterator it;
	if (_request.getMethod() == "GET")
	{
		if (ConfigFile::getTypePath(server.getRoot()+_request.getURL()) == 1)
		{
			frontpage = server.getRoot()+_request.getURL();
			it = locations.end() - 1;
		}
		else
		{
			for (it = locations.begin(); it < locations.end(); ++it)
			{
				std::cout << "        Location PATH: " << it->getPath(); 
				if (it->getPath() == _request.getURL())
				{
					std::cout << " MATCH SUCCESSFULLY !!!!" << std::endl;
					break;
				}
				else
					std::cout << " NOT MATCH !";
			std::cout << std::endl;
			}
		}
		if (it == locations.end())
		{
			std::cout << "    Location PATH: " << _request.getURL() << " NOT FOUND !!!!" << std::endl;
			std::map<short, std::string>::const_iterator it3 = server.getErrorPages().find(404);
			std::ifstream fdError((server.getRoot() + it3->second).c_str());
			std::cout << "    Value for 404 page errorr: " << server.getRoot()+it3->second << std::endl;

			if (fdError && fdError.is_open())
			{
				std::stringstream stream_binding;
				stream_binding << fdError.rdbuf();
				responseBuilder.setBody(stream_binding.str());
			}
			else
				responseBuilder.setBody("Error 404");
		}
		else
		{
			std::ifstream bodyFile(frontpage.c_str());
			std::cout << "    Index: " << frontpage << std::endl;
			if (!bodyFile || !bodyFile.is_open())
			{
				std::map<short, std::string>::const_iterator it4 = server.getErrorPages().find(404);
				std::ifstream fdError(it4->second.c_str());
				if (fdError && fdError.is_open())
				{
					std::stringstream stream_binding;
					stream_binding << fdError.rdbuf();
					responseBuilder.setBody(stream_binding.str());
				}
				else
					responseBuilder.setBody("Error 404");
			}
			else
			{
				responseBuilder.setStatusCode(200);
				std::stringstream stream_binding;
				stream_binding << bodyFile.rdbuf();
				responseBuilder.setBody(stream_binding.str());
			}
		}
		
	}
	/*
	else if (_request.getMethod() == "POST")
	{
		//Tratar POST
	}
	else if (_request.getMethod() == "DELETE")
	{
		//Tratar DELETE
	}
	else if (_request.getMethod() == "PUT")
	{
		//Tratar PUT
	}
	else if (_request.getMethod() == "HEAD")
	{
		//Tratar HEAD
	}
	else
	{
		//Error: Método no soportado
	}		
	*/	
	responseBuilder.addHeader("Content-Type", "text/html");

	// Construir la respuesta
	std::string response = responseBuilder.buildResponse();
	data.accumulatedBytes = response.size();
	data.writeBuffer = new char[data.accumulatedBytes];
	std::copy(response.begin(), response.end(), data.writeBuffer);

	while (data.writeBuffer && data.accumulatedBytes > 0)
	{
		int bytesSent = socket.send(data.writeBuffer, data.accumulatedBytes);
		std::cout << "    Bytes sent: " << bytesSent << std::endl;
		std::cout << CYAN << "****RESPONSE enviada: " << std::endl;
		std::cout << response << RESET << std::endl;
		if (bytesSent > 0)
		{
			data.accumulatedBytes -= bytesSent;
			std::memmove(data.writeBuffer, data.writeBuffer + bytesSent, data.accumulatedBytes);
		}
		else if (bytesSent == -1)
		{
			std::cerr << "Error de envio de response" << std::endl;
		}
		// Si todos los datos han sido enviados, puedes decidir vaciar completamente el buffer
		if (data.accumulatedBytes == 0)
		{
			delete[] data.writeBuffer;
			data.writeBuffer = NULL;
		}
	}
	data.responseSent = true;
}

bool ConnectionManager::isHttpRequestComplete(const std::vector<char>& buffer, size_t accumulatedBytes)
{
	accumulatedBytes = 1024;
	if (accumulatedBytes)
		accumulatedBytes = 1024;
	std::cout << "    HTTP Complete" << std::endl;
	const std::string endOfHeader = "\r\n\r\n";
	if (std::search(buffer.begin(), buffer.end(),
		endOfHeader.begin(), endOfHeader.end()) != buffer.end())
		return true;
	return false;
}

int ConnectionManager::getContentLength(const std::vector<char>& buffer, size_t accumulatedBytes)
{
	// Convertir el buffer actual a string para buscar el Content-Length
	std::string header(buffer.begin(), buffer.begin() + accumulatedBytes);
	std::size_t startPos = header.find("    Content-Length: ");
	
	if (startPos != std::string::npos)
	{
		startPos += std::string("    Content-Length: ").length();
		std::size_t endPos = header.find("\r\n", startPos);
		if (endPos != std::string::npos)
		{
			std::string contentLengthValue = header.substr(startPos, endPos - startPos);
			try
			{
				std::istringstream iss(contentLengthValue);
				int contentLength;
				if (iss >> contentLength)
					return contentLength;
				else
					std::cerr << "Throw exception" << std::endl;		
			}
			catch (const std::exception& e)
			{
			// Manejar excepción o valor no válido
			}
		}
	}
	return 0; // Retorna 0 si no se encuentra el encabezado Content-Length
}
