/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseBuilder.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/12 13:27:01 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/18 12:37:58 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ResponseBuilder.hpp"

ResponseBuilder::ResponseBuilder() : _statusCode(200) {}

void ResponseBuilder::setStatusCode(int code)
{
	_statusCode = code;
}

void ResponseBuilder::addHeader(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

void ResponseBuilder::setBody(const std::string& bodyContent)
{
	_body = bodyContent;
	std::ostringstream convert; // Crea un stringstream
    convert << _body.size(); // Inserta el valor numérico en el stream
    addHeader("Content-Length", convert.str());
}

std::string ResponseBuilder::buildResponse() const
{
	std::string response = "HTTP/1.1 ";

	switch(_statusCode)
	{
		case 200: response += "200 OK"; break;
		default: response += "500 Internal Server Error"; break;
	}

	response += "\r\n";

	for(std::map<std::string, std::string>::const_iterator it = _headers.begin();
		it != _headers.end(); ++it)
		response += it->first + ": " + it->second + "\r\n";

	// Línea en blanco para separar encabezados del cuerpo
	response += "\r\n";
	response += _body;

	return response;
}
