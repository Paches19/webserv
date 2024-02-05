/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 12:41:27 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/05 12:28:47 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

// Libraries
# include <string>
# include <map>
# include <vector>
# include <iostream>
# include <fstream>
# include <sstream>
# include <sys/stat.h>
# include <unistd.h>
# include <stdint.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <cstring>
# include <sys/socket.h>
# include <sys/stat.h>
# include <errno.h>
# include <sys/select.h>
# include <poll.h>
# include <algorithm>
# include <fcntl.h>
# include <dirent.h>

// Colours
# define RESET          "\x1B[0m"
# define RED            "\x1B[31m"
# define LIGHTRED      	"\x1B[91m"
# define YELLOW         "\x1B[33m"
# define LIGHTBLUE     	"\x1B[94m"
# define CYAN           "\x1B[36m"
# define DARKGREY      	"\x1B[90m"
# define LIGHTMAGENTA   "\x1B[95m"
# define GREEN 			"\x1B[32m"

// Macros
# define MAX_CONTENT_LENGTH 1000000

#endif // WEBSERVER_HPP