/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   connectionManager.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:42:54 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/10 13:19:29 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "connectionManager.hpp"

ConnectionManager::ConnectionManager() {}

ConnectionManager::~ConnectionManager()
{
	for (std::map<int, std::string>::iterator it = connections.begin();
		it != connections.end(); ++it)
		close(it->first);
}

void ConnectionManager::addConnection(int socketFd)
{
	connections[socketFd] = "";
	std::cout << "Nueva conexión agregada. Socket FD: " << socketFd << std::endl;
}

void ConnectionManager::removeConnection(int socketFd)
{
	std::map<int, std::string>::iterator it = connections.find(socketFd);
	if (it != connections.end())
	{
		close(socketFd);
		connections.erase(it);
		std::cout << "Conexión eliminada. Socket FD: " << socketFd << std::endl;
	}
}

void ConnectionManager::processData(int socketFd)
{
	std::cout << "Procesando datos del socket FD: " << socketFd << std::endl;
	// Aquí iría la lógica para leer y procesar datos del socket
    // Por ejemplo, leer con recv() y enviar una respuesta
}
