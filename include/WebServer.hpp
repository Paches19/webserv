#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# include <string>
# include <map>
# include <vector>
# include <algorithm>
# include <iostream>
# include <fstream>
# include <sstream>
# include <sys/stat.h>
# include <unistd.h>
# include <stdint.h>
# include <poll.h>
# include <netinet/in.h>
# include <sys/socket.h>

# define RESET          "\x1B[0m"
# define RED            "\x1B[31m"
# define LIGHTRED      	"\x1B[91m"
# define YELLOW         "\x1B[33m"
# define LIGHTBLUE     	"\x1B[94m"
# define CYAN           "\x1B[36m"
# define DARKGREY      	"\x1B[90m"
# define LIGHTMAGENTA   "\x1B[95m"
# define GREEN 			"\x1B[32m"

# define MAX_CONTENT_LENGTH 1000000

# include "ConfigParser.hpp"
# include "ConfigFile.hpp"
# include "VirtualServers.hpp"
# include "Location.hpp"
# include "Socket.hpp"
# include "ConnectionManager.hpp"
# include "HttpRequest.hpp"
# include "ResponseBuilder.hpp"
# include "ConnectionData.hpp"
# include "Server.hpp"

#endif