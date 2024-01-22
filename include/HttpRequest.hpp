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

#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <map>
# include <sys/socket.h>
# include <iostream>
# include <unistd.h>
# include <sstream>
# include <string>
# include <vector>
# include <algorithm>

class HttpRequest
{
	private:
		std::string _method;
		std::string _url;
		std::string _httpVersion;
		std::map<std::string, std::string> _headers;
		std::string _body;
		std::string _errorMessage;
		bool		_isValid;

		void parseRequest(const std::string& rawRequest);
		void parseFirstLine(const std::string& line);
		void parseHeaders(const std::string& headersStr);
	
	public:
		HttpRequest(const std::string& rawRequest);
		~HttpRequest(void);

		std::string getMethod();
		std::string getURL();
		std::string getHttpVersion();
		std::map<std::string, std::string> getHeaders();
		std::string getBody();
		
		bool isValidRequest();
		std::string errorMessage();
		void invalidRequest();
};

#endif // HTTP_REQUEST_HPP