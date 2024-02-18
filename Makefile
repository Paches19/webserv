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

# DEFINES = -D OUT=0# -D TIME=1

ifeq ($(shell uname -s),Darwin)
OS = mac
else
OS = linux
endif
PWD = $(shell pwd)

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
					printf "\n\033[0;32mLinking ...\033[0m $@"
					$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
					printf " \033[0;32mOK!\033[0m\n"

create_dir		:
					mkdir -p $(OBJ_DIR)

$(OBJ_DIR)%.o	:	$(SRC_DIR)%.cpp
					printf "\033[0;33mGenerating objects ... \033[0m$@"
					$(CXX) $(CXXFLAGS) -I$(INC_DIR) -o $@ -c $<
					printf " \033[0;32mOK!\033[0m\n"

clean			:
					printf "\n\033[0;31mDeleting objects\033[0m"
					rm -rf $(OBJ_DIR)
					printf " \033[0;32mOK!\033[0m\n"

fclean			: 	clean
					printf "\033[0;31mDeleting \033[0m $(NAME)"
					rm -rf $(NAME)
					printf " \033[0;32mOK!\033[0m\n"

re				: 	fclean all

##################################################
# TESTING
##################################################

test: test_$(OS)

test_setup: all
	@rm -rf docs/test_us/root
	@mkdir -p docs/test_us/root
	@cp docs/test_us/index/* docs/test_us/root/
	@cp docs/test_us/root/index.html docs/test_us/root/index_permission.html
	@chmod 000 docs/test_us/root/index_permission.html
	@c++ -o client docs/test_us/client.cpp

test_mac: test_setup
	@osascript -e 'tell application "Terminal" to do script "cd $(PWD) && clear && ./docs/client"'
	@osascript -e 'tell application "Terminal" to activate'
	./webserv docs/test_us/conf/webserv.conf

test_linux: test_setup
	@x-terminal-emulator --working-directory=$$(pwd) -x "./docs/client"
	./webserv docs/test_us/conf/webserv.conf

bocal: bocal_$(OS)

bocal_mac: all
	@mkdir -p docs/YoupiBanane/put_here
	@osascript -e 'tell application "Terminal" to do script "cd $(PWD) && clear && time ./test_mac/macos_tester http://localhost:8000"'
	@osascript -e 'tell application "Terminal" to activate'
	./webserv docs/test_mac/mac.conf

bocal_linux: all
	@mkdir -p docs/YoupiBanane/put_here
	@x-terminal-emulator --working-directory=$$(pwd) -x "time ./docs/test_linux/ubuntu_tester http://localhost:8000"
	./webserv docs/test_linux/linux.conf
	

.PHONY			: 	all re clean fclean create_dir

.SILENT			: 	create_dir clean fclean re $(OBJS) $(NAME) $(OBJ_DIR)%.o
