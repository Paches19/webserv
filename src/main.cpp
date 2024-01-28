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
			cluster.print();
		}
		catch (std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			return (1);
		}
		Server myServer(cluster.getServers());

		myServer.run(cluster.getServers());
		
		// // Ejemplo de solicitud GET
		// std::string getRequest = 
		// 	"GET /index.html HTTP/1.1\r\n"
		// 	"Host: www.example.com\r\n"
		// 	"User-Agent: testAgent\r\n"
		// 	"Accept: */*\r\n\r\n";

		// // Ejemplo de solicitud POST
		// std::string postRequest = 
		// 	"POST /submit-form HTTP/1.1\r\n"
		// 	"Host: www.example.com\r\n"
		// 	"Content-Length: 27\r\n"
		// 	"Content-Type: application/x-www-form-urlencoded\r\n\r\n"
		// 	"field1=value1&field2=value2\r\n";

		// // Ejemplo de solicitud DELETE
		// std::string deleteRequest = 
		// 	"DELETE   /resource HTTP/1.1\r\n"
		// 	"Host: www.example.com\r\n\r\n";

		// // Analizar cada solicitud
		// HttpRequest httpGet(getRequest);
		// std::cout << "GET Request:" << std::endl << std::endl;
		// std::cout << "Method: " << httpGet.getMethod() << std::endl;
		// std::cout << "URL: " << httpGet.getURL() << std::endl;
		// std::cout << "HTTP Version: " << httpGet.getHttpVersion() << std::endl;
		// std::cout << "Headers :" << std::endl;
		// std::map<std::string, std::string> headers = httpGet.getHeaders();
		// std::map<std::string, std::string>::const_iterator it;
		// for (it = headers.begin(); it != headers.end(); ++it)
		// 	std::cout << it->first << ": " << it->second << std::endl;
		// std::cout << std::endl;
		
		// HttpRequest httpPost(postRequest);
		// std::cout << "POST Request:" << std::endl << std::endl;
		// std::cout << "Method: " << httpPost.getMethod() << std::endl;
		// std::cout << "URL: " << httpPost.getURL() << std::endl;
		// std::cout << "HTTP Version: " << httpPost.getHttpVersion() << std::endl;
		// std::cout << "Headers :" << std::endl;
		// headers = httpPost.getHeaders();
		// for (it = headers.begin(); it != headers.end(); ++it)
		// 	std::cout << it->first << ": " << it->second << std::endl;
		// std::cout << "Body: " << httpPost.getBody() << std::endl << std::endl;
		
		// HttpRequest httpDelete(deleteRequest);
		// std::cout << "DELETE Request:" << std::endl << std::endl;
		// std::cout << "Method: " << httpDelete.getMethod() << std::endl;
		// std::cout << "URL: " << httpDelete.getURL() << std::endl;
		// std::cout << "HTTP Version: " << httpDelete.getHttpVersion() << std::endl;
		// std::cout << "Headers :" << std::endl;
		// headers = httpDelete.getHeaders();
		// for (it = headers.begin(); it != headers.end(); ++it)
		// 	std::cout << it->first << ": " << it->second << std::endl;
    }
    else 
	{
		std::cout << "Error: wrong arguments" << std::endl;
		return (1);
	}
	
    return (0);
}
