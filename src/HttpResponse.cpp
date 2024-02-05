/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/12 13:27:01 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/05 18:07:49 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

//*******************************************************************
// Constructores y destructor de la clase canónica
//*******************************************************************
HttpResponse::HttpResponse(void) { _statusCode = 200; }
HttpResponse::~HttpResponse() {}
HttpResponse::HttpResponse(const HttpResponse& copy)
{
	_statusCode = copy._statusCode;
	_headers = copy._headers;
	_body = copy._body;
}
HttpResponse& HttpResponse::operator=(const HttpResponse& rhs)
{
	if (this != &rhs)
	{
		_statusCode = rhs._statusCode;
		_headers = rhs._headers;
		_body = rhs._body;
	}
	return *this;
}

//*******************************************************************
// Setters
//*******************************************************************
void HttpResponse::setHeader(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

void HttpResponse::setBody(const std::string& bodyContent)
{
	_body = bodyContent;
}

void HttpResponse::setStatusCode(int code) {	_statusCode = code; }

//*******************************************************************
// Getters
//*******************************************************************
std::string 	HttpResponse::getBody() { return _body; }

//*******************************************************************
// Métodos de la clase
//*******************************************************************
std::string getStatusMessage(int statusCode)
{
	switch (statusCode)
	{
		case 200:
			return "OK";
		case 400:
			return "Bad Request";
		case 401:
			return "Unauthorized";
		case 403:
			return "Forbidden";
		case 404:
			return "Not Found";
		case 500:
			return "Internal Server Error";
		default:
			return "Unknown Server Error";
	}
}

std::string HttpResponse::buildResponse()
{
	std::stringstream response;

	std::cout << std::endl;
	// std::cout << "build Response: " << std::endl;
	// Obtiene el mensaje de estado correspondiente al código de estado
	std::string statusMessage = getStatusMessage(_statusCode);

	// Agrega la línea de estado (código de estado y mensaje)
	response << "HTTP/1.1 " << _statusCode << " " << statusMessage << "\r\n";
	// std::cout << "HTTP/1.1 " << _statusCode << " " << statusMessage << std::endl;

	// Agrega las cabeceras
	for (std::map<std::string, std::string>::const_iterator it =	
		_headers.begin(); it != _headers.end(); ++it)
	{
		response << it->first << ": " << it->second << "\r\n";
		// std::cout << it->first << ": " << it->second << std::endl;
	}
		

	// Agrega la longitud del cuerpo y la línea en blanco que indica el final de las cabeceras
	response << "Content-Length: " << _body.length() << "\r\n";
	response << "\r\n";

	// Agrega el cuerpo
	response << _body;
	// std::cout << "body: " << _body << std::endl << std::endl;

	return response.str();
}

