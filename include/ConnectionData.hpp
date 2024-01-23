/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionData.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 11:31:40 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/22 12:01:01 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_DATA_HPP
# define CONNECTION_DATA_HPP

# include <map>
# include <sys/socket.h>
# include <string>
# include <vector>
# include <iostream>
# include <unistd.h>
#include <sstream>
# include "Socket.hpp"
# include "HttpRequest.hpp"
# include "ResponseBuilder.hpp"

class ConnectionData
{
	public:
		std::vector<char> readBuffer;
		std::vector<char> writeBuffer;
		bool headerReceived;
		size_t accumulatedBytes;
			
		ConnectionData();
		
		ConnectionData(const ConnectionData& other);

		ConnectionData& operator=(const ConnectionData& other);
};

#endif // CONNECTION_DATA_HPP