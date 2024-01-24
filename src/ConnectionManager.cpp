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
	std::cout << "Nueva conexión agregada. Socket FD: " << socketFd << std::endl;
}

void ConnectionManager::removeConnection(Socket& socket)
{
    int socketFd = socket.getSocketFd();
    std::map<int, ConnectionData>::iterator it = connections.find(socketFd);
    if (it != connections.end())
    {
        socket.close();
        connections.erase(it);
        std::cout << "Conexión eliminada. Socket FD: " << socketFd << std::endl;
    }
    else
    {
        std::cout << "Conexión no encontrada. Socket FD: " << socketFd << std::endl;
    }
}

void ConnectionManager::readData(Socket& socket)
{
	ConnectionData& data(connections[socket.getSocketFd()]);

	std::cout << "socketLectura: " << socket.getSocketFd() << std::endl;
	// Leer datos del socket
	int bytesRead = socket.receive(&data.readBuffer[0], data.readBuffer.size());
	// std::cout << "bytesRead: " << bytesRead << std::endl;
	// std::cout << "Buff: " << std::string(data.readBuffer.begin(), data.readBuffer.end()) << std::endl;
	if (!data.readBuffer.empty())
	{
		std::cout << "ReadBuffer no vacio:" << std::endl << std::endl;
		data.accumulatedBytes += bytesRead + data.readBuffer.size(); // Añadir a la cuenta de bytes acumulados
		// int contentLength = 0;
		
		// if (!data.headerReceived && isHttpRequestComplete(data.readBuffer, data.accumulatedBytes))
		// {
		// 	std::cout << "data.headerReceived: " << data.headerReceived << std::endl;
		// 	data.headerReceived = true;
		// 	contentLength = getContentLength(data.readBuffer, data.accumulatedBytes);
		// 	if (contentLength > 0)
		// 	{
		// 		// Redimensionar readBuffer para acomodar el cuerpo de la solicitud
		// 		data.readBuffer.resize(data.accumulatedBytes + contentLength);
		// 	}
		// }
		// if (data.headerReceived && ((data.accumulatedBytes == data.readBuffer.size())
		// 	&& contentLength > 0))
		if (isHttpRequestComplete(data.readBuffer, data.accumulatedBytes))
		{
			// std::cout << "data.headerReceived: " << data.headerReceived << std::endl;
			// Procesar la solicitud completa
			HttpRequest request(std::string(data.readBuffer.begin(),
				data.readBuffer.end()));
			std::cout << "Request:" << std::endl << std::endl;
			std::cout << "Method: " << request.getMethod() << std::endl;
			std::cout << "URL: " << request.getURL() << std::endl;
			std::cout << "HTTP Version: " << request.getHttpVersion() << std::endl;
			std::cout << "Headers :" << std::endl;
			std::map<std::string, std::string> headers = request.getHeaders();
			std::map<std::string, std::string>::const_iterator it;
			for (it = headers.begin(); it != headers.end(); ++it)
				std::cout << it->first << ": " << it->second << std::endl;
			std::cout << std::endl;
			if (request.isValidRequest())
			{
				// Procesar la solicitud y preparar la respuesta
				ResponseBuilder response;
				response.buildResponse();
				// data.writeBuffer.assign(response.begin(), response.end());
			}
			else
			{
				std::cerr << "Invalid request" << std::endl;
				// Manejar solicitud inválida
				// std::string errorResponse = request.errorMessage();
				// data.writeBuffer.assign(errorResponse.begin(), errorResponse.end());
			}
			if (!data.readBuffer.empty())
				data.readBuffer.clear();
			data.readBuffer.resize(1024);
			data.accumulatedBytes = 0;
			data.headerReceived = false;
		}
	}
	// else if (bytesRead == 0) 
	// {
	// 	// Cliente cerró la conexión
	// 	removeConnection(socket);
	// } 
	// else 
	// {
	// 	// data.readBuffer.clear();
	// 	data.readBuffer.resize(1024); // Volver al tamaño inicial
	// 	data.accumulatedBytes = 0;
	// 	data.headerReceived = false;
	// }
}

void ConnectionManager::writeData(Socket& socket) 
{
	ConnectionData& data(connections[socket.getSocketFd()]);

	// Enviar datos desde el buffer de escritura
	int bytesSent = socket.send(&data.writeBuffer[0], data.writeBuffer.size());
	if (bytesSent > 0)
		data.writeBuffer.erase(data.writeBuffer.begin(), data.writeBuffer.begin()
			+ bytesSent);
	else if (bytesSent == -1)
	{
		std::cerr << "Error de envio de response" << std::endl;
	}
	// Si todos los datos han sido enviados, puedes decidir vaciar completamente el buffer
	if (data.writeBuffer.empty())
		data.writeBuffer.clear();
}

bool ConnectionManager::isHttpRequestComplete(const std::vector<char>& buffer,
	size_t accumulatedBytes)
{
	accumulatedBytes = 1024;
	if (accumulatedBytes)
		accumulatedBytes = 1024;
	std::cout << "http COmplete" << std::endl;
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
	std::size_t startPos = header.find("Content-Length: ");
	
	if (startPos != std::string::npos)
	{
		startPos += std::string("Content-Length: ").length();
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
