/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/12 13:25:48 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/01 18:18:07 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include "WebServer.hpp"

class HttpResponse
{
	private:
		int _statusCode;
		std::map<std::string, std::string> _headers;
		std::string _body;

	public:
		HttpResponse();
		~HttpResponse();
		HttpResponse(const HttpResponse& copy);
		HttpResponse& operator=(const HttpResponse& rhs);

		void setStatusCode(int code);
		void addHeader(const std::string& key, const std::string& value);
		void setBody(const std::string& bodyContent);
		std::string buildResponse();
};

#endif // HTTPRESPONSE_HPP
