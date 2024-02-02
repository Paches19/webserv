/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:42:54 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/01 18:20:00 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionManager.hpp"

ConnectionManager::ConnectionManager() {}

ConnectionManager::~ConnectionManager() {}

ConnectionManager::ConnectionManager(const ConnectionManager& other)
{
	connections = other.connections;
}

ConnectionManager& ConnectionManager::operator=(const ConnectionManager& other)
{
	if (this != &other)
		connections = other.connections;
	return *this;
}

void ConnectionManager::addConnection(Socket& socket)
{
	int socketFd = socket.getSocketFd();
	ConnectionData connData;

	connections.insert(std::make_pair(socketFd, connData));
	std::cout << " agregada con Socket FD = " << socketFd << std::endl;
}

void ConnectionManager::removeConnection(Socket& socket, int i,
			std::vector<struct pollfd>& _pollFds, std::vector<Socket *>& _clientSockets)
{
	int socketFd = _pollFds[i].fd;
	for (size_t j = 0; j < _clientSockets.size(); ++j)
	{
		if (_clientSockets[j]->getSocketFd() == socketFd)
		{
			std::cout << "Client socket erased " << _clientSockets[j]->getSocketFd() << std::endl;
			_clientSockets.erase(_clientSockets.begin() + j);
		}
	}
	_pollFds.erase(_pollFds.begin() + i);
						
    std::map<int, ConnectionData>::iterator it = connections.find(socketFd);
    if (it != connections.end())
    {
        socket.close();
        connections.erase(it);
        std::cout << "\nConexión eliminada. Socket FD = " << socketFd << std::endl;
    }
    else
    {
        std::cout << "\nConexión no encontrada. Socket FD = " << socketFd << std::endl;
    }
}

HttpRequest ConnectionManager::readData(Socket& socket, int i,
			std::vector<struct pollfd> &_pollFds, std::vector<Socket *> &_clientSockets)
{
	ConnectionData data(connections[socket.getSocketFd()]);

	std::cout << "\nSocket de lectura: " << socket.getSocketFd() << std::endl;
	// Leer datos del socket
	int bytesRead = socket.receive(&data.readBuffer[0], data.readBuffer.size());
	std::cout << "    Bytes Read: " << bytesRead << std::endl;

	if (bytesRead > 0)
	{
		data.accumulatedBytes += bytesRead; // Añadir a la cuenta de bytes acumulados

		if (isHttpRequestComplete(data.readBuffer, data.accumulatedBytes))
		{
			// Procesar la solicitud completa
			HttpRequest request(std::string(data.readBuffer.begin(), data.readBuffer.end()));

			std::cout << "\nREQUEST recibida: " << std::endl;
			std::cout << YELLOW << "Method: " << request.getMethod() << std::endl;
			std::cout << "URL: " << request.getURL() << std::endl;
			std::cout << "HTTP Version: " << request.getHttpVersion() << std::endl;
			std::cout << "Headers: " << std::endl;
			std::map<std::string, std::string> headers = request.getHeaders();
			std::map<std::string, std::string>::const_iterator it;
			for (it = headers.begin(); it != headers.end(); ++it)
				std::cout << it->first << ": " << it->second << std::endl;
			std::cout << RESET << std::endl;
			if (request.isValidRequest())
			{
				data.responseSent = false;
				request.setValidRequest(true);
				request.setCompleteRequest(true);
				_pollFds[i].events = POLLOUT;
			}
			else
			{
				std::cerr << "    Invalid request" << std::endl;
				return request;
			}
			data.readBuffer.clear();
			data.readBuffer.resize(1024);
			data.accumulatedBytes = 0;
			data.headerReceived = false;
			connections[socket.getSocketFd()] = data;
			return request;
		}
		connections[socket.getSocketFd()] = data;
	}
	else if (bytesRead == 0) 
	{
		this->removeConnection(socket, i, _pollFds, _clientSockets);
		HttpRequest invalidRequest(std::string(data.readBuffer.begin(), data.readBuffer.end()));
		return invalidRequest;
	}
	else 
	{
		this->removeConnection(socket, i, _pollFds, _clientSockets);
		HttpRequest invalidRequest(std::string(data.readBuffer.begin(), data.readBuffer.end()));
		return invalidRequest;
	}
	HttpRequest incompleteRequest;
	incompleteRequest.setValidRequest(true);
	return incompleteRequest;
}

void ConnectionManager::writeData(Socket& socket, VirtualServers &server) 
{
	// while (data.writeBuffer && data.accumulatedBytes > 0)
	// {
	// 	int bytesSent = socket.send(data.writeBuffer, data.accumulatedBytes);
	// 	std::cout << "\nRESPONSE enviada: " << std::endl;
	// 	std::cout << CYAN << response << RESET << std::endl;
	// 	if (bytesSent > 0)
	// 	{
	// 		data.accumulatedBytes -= bytesSent;
	// 		std::memmove(data.writeBuffer, data.writeBuffer + bytesSent, data.accumulatedBytes);
	// 	}
	// 	else if (bytesSent == -1)
	// 	{
	// 		std::cerr << "Error de envio de response" << std::endl;
	// 	}
	// 	// Si todos los datos han sido enviados, puedes decidir vaciar completamente el buffer
	// 	if (data.accumulatedBytes == 0)
	// 	{
	// 		delete[] data.writeBuffer;
	// 		data.writeBuffer = NULL;
	// 	}
	// }
	// data.responseSent = true;
}

bool ConnectionManager::isHttpRequestComplete(const std::vector<char>& buffer, size_t accumulatedBytes)
{
	accumulatedBytes = 1024;
	if (accumulatedBytes)
		accumulatedBytes = 1024;
	std::cout << "    Http complete !" << std::endl;
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
