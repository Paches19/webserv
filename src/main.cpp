/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/10 12:34:17 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/22 16:44:17 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ConfigParser.hpp"
#include "WebServer.hpp"

int main(int argc, char **argv) 
{
	if (argc == 1 || argc == 2)
	{
		ConfigParser	cluster;
		try 
		{
			std::string		config;
			
			// Configuration file as argument or default path
			config = (argc == 1 ? "config/default.conf" : argv[1]);
			cluster.initParser(config);
			//cluster.print();
		}
		catch (std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			return (1);
		}
		Server myServer(cluster.getServers());
		myServer.run(cluster.getServers());
    }
    else 
	{
		std::cout << "Error: wrong arguments" << std::endl;
		return (1);
	}
	
    return (0);
}
