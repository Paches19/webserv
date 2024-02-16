/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 12:40:51 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/22 12:49:55 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "Webserver.hpp"
# include "VirtualServers.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"

class CgiHandler
{
	private:
		std::map<std::string, std::string>	_env;
		std::string	_body;
		void		_initEnv(HttpRequest &request, const Location &config, VirtualServers &server);
		char**		_getEnvAsCstrArray() const;

	public:
		CgiHandler(void);
		// sets up env according to the request
		CgiHandler(HttpRequest &request, const Location &config, VirtualServers &server);
		CgiHandler(CgiHandler const &src);
		virtual ~CgiHandler();
		CgiHandler	&operator=(CgiHandler const &src);
		// executes cgi and returns body or error
		std::string	executeCgi(std::string& scriptName);
};

/*
Some CGI environment variables for Chrome browser:
	HTTP_ACCEPT: Specifies the media types that the client can process, such as text/html or image/png.
	HTTP_ACCEPT_LANGUAGE: Indicates the natural languages that the client can understand.
	HTTP_USER_AGENT: Identifies the user agent (browser) making the request, including information about the browser type and version, as well as the operating system.
	HTTP_REFERER: Contains the URL of the page that referred the user to the current page.
	HTTP_COOKIE: Holds any cookies that have been sent by the server to the client.
	HTTP_HOST: Provides the domain name of the server.
	REMOTE_ADDR: Represents the IP address of the client.
	REMOTE_PORT: Specifies the port number on the client.
	REQUEST_METHOD: Indicates the HTTP request method, such as GET or POST.
	QUERY_STRING: Holds the query string portion of the URL for a GET request.
	CONTENT_LENGTH: Specifies the size of the message body for requests with a body, such as POST requests.
	CONTENT_TYPE: Describes the type of data in the body of the request, particularly relevant for POST requests.
	PATH_INFO: Contains the path information that follows the actual script name in the URL.
*/

#endif
