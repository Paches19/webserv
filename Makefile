# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/01/12 11:30:21 by adpachec          #+#    #+#              #
#    Updated: 2024/01/22 12:58:35 by adpachec         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#Program name
NAME	=	webserv

SRC_DIR	= 	src/
INC_DIR	= 	include/
OBJ_DIR	= 	obj/

#Sources 
SRC		=	ConfigFile.cpp \
			ConfigParser.cpp \
			ConnectionData.cpp \
			ConnectionManager.cpp \
			HttpRequest.cpp \
			HttpResponse.cpp \
			Location.cpp \
			main.cpp \
			Server.cpp \
			Socket.cpp \
			VirtualServers.cpp

SRCS 	= 	$(addprefix $(SRC_DIR), $(SRC))

#Headers
HEADERS = 	$(INC_DIR)HttpRequest.hpp \
			$(INC_DIR)HttpResponse.hpp \
			$(INC_DIR)Server.hpp \
			$(INC_DIR)Socket.hpp \
			$(INC_DIR)ConnectionManager.hpp \
			$(INC_DIR)ConnectionData.hpp \
			$(INC_DIR)WebServer.hpp

#Objects
OBJ		= 	$(SRC:.cpp=.o)
OBJS	= 	$(addprefix $(OBJ_DIR), $(OBJ))

#Flags
CXX		= 	c++
CXXFLAGS= 	-Wall -Wextra -Werror -std=c++98 -fsanitize=address

all		: 	create_dir $(NAME)

$(NAME)	: 	$(OBJS) $(HEADERS)
			$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

create_dir:
			@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
			$(CXX) $(CXXFLAGS) -I$(INC_DIR) -o $@ -c $<

clean	:
			@rm -rf $(OBJ_DIR)

fclean	: 	clean
			@rm -rf $(NAME)

re		: 	fclean all

.PHONY	: 	all re clean fclean create_dir
