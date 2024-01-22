/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/12 10:49:30 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/22 11:55:34 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

HttpRequest::HttpRequest(const std::string& rawRequest)
{
	this->_isValid = true;
	if (rawRequest.empty())
		invalidRequest();
	else
		parseRequest(rawRequest);
}

HttpRequest::~HttpRequest() {};

void HttpRequest::parseRequest(const std::string& rawRequest)
{
	std::istringstream requestStream(rawRequest);
	std::string line;

	// Parsear la primera línea
	getline(requestStream, line);
	parseFirstLine(line);
	if (!this->_isValid)
		return ;

	// Parsear las cabeceras
	std::string headersStr;
	while (getline(requestStream, line) && line[0] && line != "\r")
		headersStr += line + "\n";
	parseHeaders(headersStr);
	if (!this->_isValid)
		return ;

	// Parsear el cuerpo (si existe)
	if (this->_method == "POST")
	{
		std::string bodyStr;
		while (getline(requestStream, line))
			_body += line + "\n";
	}
}

void HttpRequest::parseFirstLine(const std::string& line)
{
	std::istringstream lineStream(line);
	lineStream >> _method >> _url >> _httpVersion;
	if (lineStream.fail() || (_method != "GET" && _method != "POST" && _method != "DELETE") ||
		_url == "" || _httpVersion == "")
		return invalidRequest();
}

void HttpRequest::parseHeaders(const std::string& headersStr)
{
	std::istringstream headersStream(headersStr);
	std::string line;
	while (getline(headersStream, line))
	{
		std::size_t colonPos = line.find(':');
		if (colonPos != std::string::npos && colonPos > 0)
		{
			std::string headerName = line.substr(0, colonPos);
			std::string headerValue = line.substr(colonPos + 2); // +2 para saltar el espacio después de los dos puntos
			if (headerName == "" || headerValue == "")
				return invalidRequest();
			_headers[headerName] = headerValue;
		}
		else
			return invalidRequest();
	}
}

std::string HttpRequest::getMethod()
{
	return (this->_method);
}

std::string HttpRequest::getURL()
{
	return (this->_url);
}

std::string HttpRequest::getHttpVersion()
{
	return (this->_httpVersion);
}

std::map<std::string, std::string> HttpRequest::getHeaders()
{
	return (this->_headers);
}
std::string HttpRequest::getBody()
{
	return (this->_body);
}

bool HttpRequest::isValidRequest()
{
	return (this->_isValid);
}

std::string HttpRequest::errorMessage()
{
	return (this->_errorMessage);
}

void HttpRequest::invalidRequest()
{
	this->_isValid = false;
	this->_errorMessage = "Bad Request";
}