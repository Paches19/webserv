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

# Program name
NAME			=	webserv

# Folders
SRC_DIR			= 	src/
INC_DIR			= 	include/
OBJ_DIR			= 	obj/

# Sources 
SRC				=	ConfigFile.cpp \
					ConfigParser.cpp \
					ConnectionData.cpp \
					ConnectionManager.cpp \
					HttpRequest.cpp \
					HttpResponse.cpp \
					Location.cpp \
					main.cpp \
					Server.cpp \
					Socket.cpp \
					VirtualServers.cpp \
					CgiHandler.cpp

SRCS 			= 	$(addprefix $(SRC_DIR), $(SRC))

# Headers
HEADERS 		=	$(INC_DIR)HttpRequest.hpp \
					$(INC_DIR)HttpResponse.hpp \
					$(INC_DIR)Server.hpp \
					$(INC_DIR)Socket.hpp \
					$(INC_DIR)ConnectionManager.hpp \
					$(INC_DIR)ConnectionData.hpp \
					$(INC_DIR)WebServer.hpp \
					$(INC_DIR)CgiHandler.hpp

# Objects
OBJ				= 	$(SRC:.cpp=.o)
OBJS			= 	$(addprefix $(OBJ_DIR), $(OBJ))

# Flags
CXX				= 	c++
CXXFLAGS		= 	-Wall -Wextra -Werror -std=c++98 -fsanitize=address

# Rules
all				: 	create_dir $(NAME)

$(NAME)			: 	$(OBJS) $(HEADERS)
					echo "Linking $@"
					$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

create_dir		:
					mkdir -p $(OBJ_DIR)

$(OBJ_DIR)%.o	:	$(SRC_DIR)%.cpp
					echo "Compiling $<"
					$(CXX) $(CXXFLAGS) -I$(INC_DIR) -o $@ -c $<

clean			:
					echo "Cleaning objects..."
					rm -rf $(OBJ_DIR)

fclean			: 	clean
					echo "Cleaning $(NAME)..."
					rm -rf $(NAME)

re				: 	fclean all

.PHONY			: 	all re clean fclean create_dir

.SILENT			: 	create_dir clean fclean re $(OBJS) $(NAME) $(OBJ_DIR)%.o
