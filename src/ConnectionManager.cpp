/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:42:54 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/22 12:03:30 by adpachec         ###   ########.fr       */
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

	// Leer datos del socket
	int bytesRead = socket.receive(&data.readBuffer[0], data.readBuffer.size());
	if (bytesRead > 0)
	{
		data.accumulatedBytes += bytesRead; // Añadir a la cuenta de bytes acumulados
		int contentLength = 0;
		
		if (!data.headerReceived && isHttpRequestComplete(data.readBuffer, data.accumulatedBytes))
		{
			data.headerReceived = true;
			contentLength = getContentLength(data.readBuffer, data.accumulatedBytes);
			if (contentLength > 0)
			{
				// Redimensionar readBuffer para acomodar el cuerpo de la solicitud
				data.readBuffer.resize(data.accumulatedBytes + contentLength);
			}
		}
		if (data.headerReceived && ((data.accumulatedBytes == data.readBuffer.size())
			&& contentLength > 0))
		{
			// Procesar la solicitud completa
			HttpRequest request(std::string(data.readBuffer.begin(),
				data.readBuffer.end()));

			if (request.isValidRequest())
			{
				// Procesar la solicitud y preparar la respuesta
				ResponseBuilder response;
				response.buildResponse();
				// data.writeBuffer.assign(response.begin(), response.end());
			}
			else
			{
				// Manejar solicitud inválida
				std::string errorResponse = request.errorMessage();
				data.writeBuffer.assign(errorResponse.begin(), errorResponse.end());
			}
			data.readBuffer.clear();
			data.readBuffer.resize(1024); // Volver al tamaño inicial
			data.accumulatedBytes = 0;
			data.headerReceived = false;
		}
	}
	else if (bytesRead == 0) 
	{
		// Cliente cerró la conexión
		removeConnection(socket);
	} 
	else 
	{
		// Error en la lectura
	}
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
		// Error en el envío
		// Manejar el error
	}
	// Si todos los datos han sido enviados, puedes decidir vaciar completamente el buffer
	if (data.writeBuffer.empty())
		data.writeBuffer.clear();
}

bool ConnectionManager::isHttpRequestComplete(const std::vector<char>& buffer,
	size_t accumulatedBytes)
{
	const std::string endOfHeader = "\r\n\r\n";
	return std::search(buffer.begin(), buffer.begin() + accumulatedBytes,
		endOfHeader.begin(), endOfHeader.end()) != buffer.begin() + accumulatedBytes;
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