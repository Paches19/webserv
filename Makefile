NAME 	= 	webserv

SRC_DIR	= 	src/
INC_DIR	= 	include/
OBJ_DIR	= 	obj/

#Sources 
SRC		=	HttpRequest.cpp \
			ResponseBuilder.cpp \
			Socket.cpp \
			ConnectionManager.cpp \
			ConnectionData.cpp \
			ConfigFile.cpp \
			ConfigParser.cpp \
			VirtualServers.cpp \
			Location.cpp \
			Server.cpp \
			main.cpp
		
SRCS 	= 	$(addprefix $(SRC_DIR), $(SRC))

#Headers
HEADERS = 	$(INC_DIR)WebServer.hpp

#Objects
OBJ		= $(SRC:.cpp=.o)
OBJS	= $(addprefix $(OBJ_DIR), $(OBJ))

#Flags
CXX 	= 	c++
FLAGS 	= 	-Wall -Wextra -Werror -std=c++98 -g3 
RM 		=	 rm -rf

#Colours
GREEN	=	\e[92;5;118m
YELLOW	=	\e[93;5;226m
GRAY	=	\e[33;2;37m
RESET	=	\e[0m
CURSIVE	=	\e[33;3m

#Rules
all		: 	create_dir $(NAME)

$(NAME) :	$(OBJS) $(HEADERS)
			printf "$(CURSIVE)$(GRAY) 	- Linking $(NAME)... $(RESET)\n"
			$(CXX) $(FLAGS) $(OBJS) $^ -o $(NAME)
			printf "$(GREEN)    - Executable ready.\n$(RESET)"

create_dir:
			@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(HEADERS)
			printf "$(CURSIVE)$(GRAY) 	- Compiling $@... $(RESET)\n"
			$(CXX) $(FLAGS) -I$(INC_DIR) -c $< -o $@


clean	:
			$(RM) $(OBJ_DIR)
			printf "$(YELLOW)    - Object removed.$(RESET)\n"

fclean	:	clean
			$(RM) $(NAME)
			printf "$(YELLOW)    - Executable removed.$(RESET)\n"

re		: 	fclean all

.PHONY	: 	all clean fclean re

.SILENT	: 	clean fclean all $(NAME) $(OBJS)
