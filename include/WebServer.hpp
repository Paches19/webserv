#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

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


# define RESET          "\x1B[0m"
# define RED            "\x1B[31m"
# define LIGHT_RED      "\x1B[91m"
# define WHITE          "\x1B[37m"
# define BLINK           "\x1b[5m"
# define YELLOW         "\x1B[33m"
# define LIGHT_BLUE     "\x1B[94m"
# define CYAN           "\x1B[36m"
# define DARK_GREY      "\x1B[90m"
# define LIGHTMAGENTA   "\x1B[95m"

# define MAX_CONTENT_LENGTH 1000000

# include "ConfigParser.hpp"
# include "ConfigFile.hpp"
# include "VirtualServers.hpp"
# include "Location.hpp"

#endif