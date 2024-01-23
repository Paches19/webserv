/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionData.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 11:32:20 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/22 12:00:50 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionData.hpp"

ConnectionData::ConnectionData() : readBuffer(1024),
	writeBuffer(1024),  headerReceived(false), accumulatedBytes(0) {}

ConnectionData::ConnectionData(const ConnectionData& other)
{
	readBuffer = other.readBuffer;
	writeBuffer = other.writeBuffer;
	headerReceived = other.headerReceived;
	accumulatedBytes = other.accumulatedBytes;
}

ConnectionData& ConnectionData::operator=(const ConnectionData& other)
{
	if (this != &other)
	{
		readBuffer = other.readBuffer;
		writeBuffer = other.writeBuffer;
		headerReceived = other.headerReceived;
		accumulatedBytes = other.accumulatedBytes;
	}
	return *this;
}
