/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/12 10:44:37 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/16 13:16:35 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include "WebServer.hpp"
# include "HttpRequest.hpp"

class HttpRequest
{
	private:
		std::string _method;
		std::string _url;
		std::string _httpVersion;
		std::string _body;
		std::string _errorMessage;
		bool		_isValid;
		bool		_isComplete;
		std::map<std::string, std::string> _headers;

		void parseRequest(const std::string& rawRequest);
		void parseFirstLine(const std::string& line);
		void parseHeaders(const std::string& headersStr);
	
	public:
		HttpRequest();
		HttpRequest(const std::string& rawRequest);
		~HttpRequest();
		HttpRequest(const HttpRequest& copy);
		HttpRequest& operator=(const HttpRequest& rhs);

		std::string getMethod();
		std::string getHost();
		std::string getURL();
		std::string getHttpVersion();
		std::string getBody();
		std::map<std::string, std::string> getHeaders();

		void		setValidRequest(bool validity);
		void		setCompleteRequest(bool complete);
		
		bool 		isValidRequest();
		bool 		isCompleteRequest();

		std::string	errorMessage();
		void		invalidRequest();
};

#endif // HTTPREQUEST_HPP