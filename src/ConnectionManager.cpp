/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:42:54 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/05 17:22:44 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionManager.hpp"

//*******************************************************************
// Constructores y destructor de la clase canónica
//*******************************************************************
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

//*******************************************************************
// Métodos de la clase
//*******************************************************************
void ConnectionManager::addConnection(Socket& socket)
{
	int socketFd = socket.getSocketFd();
	ConnectionData connData;

	connections.insert(std::make_pair(socketFd, connData));
	std::cout << "    Connection added. Socket FD: " << socketFd << std::endl;
}

void ConnectionManager::removeConnection(Socket& socket, int i,
			std::vector<struct pollfd>& _pollFds, std::vector<Socket *>& _clientSockets)
{
	int socketFd = _pollFds[i].fd;
	for (size_t j = 0; j < _clientSockets.size(); ++j)
	{
		if (_clientSockets[j]->getSocketFd() == socketFd)
		{
			std::cout << "Client socket deleted with FD: " << _clientSockets[j]->getSocketFd() << std::endl;
			_clientSockets.erase(_clientSockets.begin() + j);
		}
	}
	_pollFds.erase(_pollFds.begin() + i);
						
    std::map<int, ConnectionData>::iterator it = connections.find(socketFd);
    if (it != connections.end())
    {
        socket.close();
        connections.erase(it);
        std::cout << "Connection deleted. Socket FD = " << socketFd << std::endl;
    }
    else
    {
        //std::cout << "\nConexión no encontrada. Socket FD = " << socketFd << std::endl;
    }
}

HttpRequest ConnectionManager::readData(Socket& socket, int i,
			std::vector<struct pollfd> &_pollFds, std::vector<Socket *> &_clientSockets)
{
	ConnectionData* data(&connections[socket.getSocketFd()]);

	std::cout << "\nSocket de lectura: " << socket.getSocketFd() << std::endl;

	//Si no hay hueco en el buffer aumentamos tamaño
	if (data->readBuffer.size() - data->accumulatedBytes == 0)
		data->readBuffer.resize(data->readBuffer.size() + 1024);
	if (!data->readBuffer.empty())
	{
		std::string headers(data->readBuffer.begin(), data->readBuffer.end());
		size_t contentLength = getContentLength(headers);
		if (contentLength > 0)
			data->readBuffer.reserve(data->readBuffer.size() + contentLength);
	}

	// Leer datos del socket
	int bytesRead = socket.receive(&data->readBuffer[0], data->readBuffer.size(), data->accumulatedBytes);
	// std::cout << "Bytes Read: " << bytesRead << std::endl;
	// std::cout << "ReadBuffer: " << std::string (data->readBuffer.begin(), data->readBuffer.end())  << std::endl;
	// data->responseSent = false;

	if (bytesRead > 0)
	{
		data->accumulatedBytes += bytesRead; // Añadir a la cuenta de bytes acumulados

		if (isHttpRequestComplete(data->readBuffer, data->accumulatedBytes))
		{
			// Procesar la solicitud completa
			HttpRequest request(std::string(data->readBuffer.begin(), data->readBuffer.end()));

			std::cout << "\n***** REQUEST *****" << std::endl;
			std::cout << YELLOW << "Method: " << request.getMethod() << std::endl;
			std::cout << "URL: " << request.getURL() << std::endl;
			std::cout << "HTTP Version: " << request.getHttpVersion() << std::endl;
			std::cout << "Headers: " << std::endl;
			std::map<std::string, std::string> headers = request.getHeaders();
			std::map<std::string, std::string>::const_iterator it;
			for (it = headers.begin(); it != headers.end(); ++it)
				std::cout << "   " << it->first << ": " << it->second << std::endl;
			std::cout << RESET << std::endl;
			if (request.getIsValidRequest())
			{
				connections[socket.getSocketFd()].responseSent = false;
				request.setValidRequest(true);
				request.setCompleteRequest(true);
				_pollFds[i].events = POLLOUT | POLLERR | POLLHUP;
				data->readBuffer.clear();
				data->readBuffer.resize(1024);
				data->accumulatedBytes = 0;
				data->headerReceived = false;
				connections[socket.getSocketFd()] = *data;
				return request;
			}
			else
			{
				std::cerr << "    Invalid request" << std::endl;
				data->readBuffer.clear();
				data->readBuffer.resize(1024);
				data->accumulatedBytes = 0;
				data->headerReceived = false;
				request.setValidRequest(false);
				return request;
			}
		}
		connections[socket.getSocketFd()] = *data;
	}
	else
	{
		this->removeConnection(socket, i, _pollFds, _clientSockets);
		HttpRequest invalidRequest;
		invalidRequest.setValidRequest(false);
		return invalidRequest;
	}
	HttpRequest incompleteRequest;
	return incompleteRequest;
}

void ConnectionManager::writeData(Socket& socket, int i, HttpResponse &response,
	std::vector<struct pollfd> &_pollFds) 
{	
	ConnectionData data(connections[socket.getSocketFd()]);
	// if (data.responseSent == true)
	// 	return ;

	std::string responseStr = response.buildResponse();

	data.writeBuffer = new char[responseStr.length() + 1];
	std::strcpy(data.writeBuffer, responseStr.c_str());
	data.accumulatedBytes = responseStr.length();
	
	//std::cout << "\nENTRO writeData" << std::endl;
	
	while (data.writeBuffer && data.accumulatedBytes > 0)
	{
		int bytesSent = socket.send(data.writeBuffer, data.accumulatedBytes);
		std::cout << "\n***** RESPONSE *****" << std::endl;
		std::cout << CYAN << responseStr << RESET << std::endl;
		if (bytesSent > 0)
		{
			data.accumulatedBytes -= bytesSent;
			std::memmove(data.writeBuffer, data.writeBuffer + bytesSent, data.accumulatedBytes);
		}
		else if (bytesSent == -1)
		{
			if ( data.writeBuffer)
				delete[] data.writeBuffer;
			data.writeBuffer = NULL;
			std::cerr << "Error sending the response" << std::endl;
		}
		// Si todos los datos han sido enviados, puedes decidir vaciar completamente el buffer
		if (data.accumulatedBytes == 0)
		{
			if ( data.writeBuffer)
				delete[] data.writeBuffer;
			data.writeBuffer = NULL;
		}
	}
	connections[socket.getSocketFd()].responseSent = true;
	_pollFds[i].events = POLLIN | POLLERR | POLLHUP;
}

bool ConnectionManager::isHttpRequestComplete(const std::vector<char>& buffer, size_t accumulatedBytes)
{
	const std::string endOfHeader = "\r\n\r\n";

	std::vector<char>::const_iterator endOfHeaderPos =
		std::search(buffer.begin(), buffer.end(), endOfHeader.begin(), endOfHeader.end());

	if (endOfHeaderPos != buffer.end())
	{
		// Encabezados completos recibidos, calcular el tamaño total esperado de la solicitud
		std::string header(buffer.begin(), endOfHeaderPos + endOfHeader.length());
		size_t contentLength = getContentLength(header);
		// Calcular el total de bytes recibidos hasta ahora y actualizar accumulatedBytes
		size_t headersLength = endOfHeaderPos + endOfHeader.length() - buffer.begin();
		accumulatedBytes = headersLength + contentLength;

		// Comprobar si hemos recibido todo el cuerpo de la solicitud
		return buffer.size() >= accumulatedBytes;
	}
	return false;
}

int ConnectionManager::getContentLength(const std::string& header)
{
	// Convertir el buffer actual a string para buscar el Content-Length
	std::size_t startPos = header.find("Content-Length: ");
	
	if (startPos != std::string::npos)
	{
		startPos += std::string("Content-Length: ").length();
		std::size_t endPos = header.find("\r\n", startPos);
		if (endPos != std::string::npos)
		{
			std::string contentLengthValue = header.substr(startPos, endPos - startPos);
			std::istringstream iss(contentLengthValue);
			int contentLength;
			if (iss >> contentLength)
				return contentLength;
		}
	}
	return 0;
}
