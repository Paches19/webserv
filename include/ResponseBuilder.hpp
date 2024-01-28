/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseBuilder.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/12 13:25:48 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/18 12:38:33 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSEBUILDER_HPP
# define RESPONSEBUILDER_HPP

# include "WebServer.hpp"
# include "ResponseBuilder.hpp"

class ResponseBuilder
{
	private:
		int _statusCode;
		std::map<std::string, std::string> _headers;
		std::string _body;

	public:
		ResponseBuilder();

		void setStatusCode(int code);
		void addHeader(const std::string& key, const std::string& value);
		void setBody(const std::string& bodyContent);

		std::string buildResponse();
};

#endif // RESPONSE_BUILDER_HPP
