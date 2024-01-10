/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:38:27 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/10 13:17:39 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

Server::Server() : serverSocket(-1) {}

Server::~Server()
{
	if (serverSocket != -1)
		close(serverSocket);
}

void Server::initialize()
{
	std::cout << "Inicializando servidor..." << std::endl;
	// Crear socket
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0)
	{
		std::cerr << "Error al crear el socket" << std::endl;
		return ;
	}

	// Configurar el socket y enlazarlo a un puerto
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);  // Puerto donde escucha el servidor
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	std::fill((char *)&serverAddr.sin_zero, (char *)&serverAddr.sin_zero + sizeof(serverAddr.sin_zero), 0);

	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		std::cerr << "Error en bind de socket : " << serverSocket << std::endl;
		close(serverSocket);
		return;
	}

	// Poner el socket en modo de escucha
	if (listen(serverSocket, 10) < 0)
	{ // 10 es el número máximo de conexiones pendientes
		std::cerr << "Error en listen" << serverSocket << std::endl;
		close(serverSocket);
		return;
	}

	std::cout << "Servidor inicializado y escuchando en el puerto 8080" << std::endl;
}

void Server::run()
{
	std::cout << "Servidor en ejecución..." << std::endl;
	while (true)
	{

	}
}
