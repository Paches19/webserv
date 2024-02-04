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

//*******************************************************************
// Constructores y destructor de la clase canónica
//*******************************************************************
HttpRequest::HttpRequest(const std::string& rawRequest)
{
	this->_isValid = true;
	this->_isComplete = true;
	if (rawRequest.empty())
		invalidRequest();
	else
		_parseRequest(rawRequest);
}
HttpRequest::HttpRequest() { this->_isValid = false; this->_isComplete = false; }
HttpRequest::HttpRequest(const HttpRequest& copy) {	*this = copy; }
HttpRequest& HttpRequest::operator=(const HttpRequest& rhs)
{
	if (this != &rhs)
	{
		this->_method = rhs._method;
		this->_url = rhs._url;
		this->_httpVersion = rhs._httpVersion;
		this->_headers = rhs._headers;
		this->_body = rhs._body;
		this->_errorMessage = rhs._errorMessage;
		this->_isValid = rhs._isValid;
		this->_isComplete = rhs._isComplete;
	}
	return (*this);
}
HttpRequest::~HttpRequest() {};

//*******************************************************************
// Getters
//*******************************************************************
std::string HttpRequest::getMethod() { return (this->_method); }

std::string HttpRequest::getURL() {	return (this->_url); }

std::string HttpRequest::getHttpVersion() {	return (this->_httpVersion); }

std::map<std::string, std::string> HttpRequest::getHeaders() { return (this->_headers); }

std::string HttpRequest::getBody() { return (this->_body); }

std::string HttpRequest::getHost()
{
	std::map<std::string, std::string>::iterator it = _headers.find("Host");
	if (it != _headers.end())
		return it->second;
	return "";
}
bool HttpRequest::getIsValidRequest() { return (this->_isValid); }

bool HttpRequest::getIsCompleteRequest() { return (this->_isComplete); }

std::string HttpRequest::getErrorMessage() { return (this->_errorMessage); }

//*******************************************************************
// Setters
//*******************************************************************
void	HttpRequest::setValidRequest(bool validity) { this->_isValid = validity; }

void	HttpRequest::setCompleteRequest(bool complete) { this->_isComplete = complete; }

//*******************************************************************
// Métodos de la clase
//*******************************************************************
void HttpRequest::_parseRequest(const std::string& rawRequest)
{
	std::istringstream requestStream(rawRequest);
	std::string line;

	// Parsear la primera línea
	getline(requestStream, line);
	_parseFirstLine(line);
	if (!this->_isValid)
		return ;

	// Parsear las cabeceras
	std::string headersStr;
	while (getline(requestStream, line) && line[0] && line != "\r")
		headersStr += line + "\n";
	_parseHeaders(headersStr);
	if (!this->_isValid)
		return ;

	// Parsear el cuerpo (si existe)
	if (this->_method == "GET")
	{
		std::string bodyStr;
		while (getline(requestStream, line))
			_body += line + "\n";
	}
}

void HttpRequest::_parseFirstLine(const std::string& line)
{
	std::istringstream lineStream(line);
	lineStream >> _method >> _url >> _httpVersion;
	if (lineStream.fail() || (_method != "GET" && _method != "POST" && _method != "DELETE") ||
		_url == "" || _httpVersion == "")
		return invalidRequest();
}

void HttpRequest::_parseHeaders(const std::string& headersStr)
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

void HttpRequest::invalidRequest()
{
	this->_isValid = false;
	this->_errorMessage = "Bad Request";
}
