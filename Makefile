NAME 	= 	webserv

SRCS 	= 	src/main.cpp \
			src/ConfigFile.cpp \
			src/ConfigParser.cpp

HEADERS	=	include/WebServer.hpp

OBJS 	= 	$(SRCS:.cpp=.o)

CXX 	= 	c++

FLAGS 	= 	-Wall -Wextra -Werror -std=c++98 -g3 

RM 		=	 rm -rf

GREEN	=	\e[92;5;118m
YELLOW	=	\e[93;5;226m
GRAY	=	\e[33;2;37m
RESET	=	\e[0m
CURSIVE	=	\e[33;3m

all		: 	$(NAME)

%.o		:	%.cpp $(HEADERS)
			printf "$(CURSIVE)$(GRAY) 	- Compiling $@... $(RESET)\n"
			$(CXX) $(FLAGS) -c $< -o $@

$(NAME) :	$(OBJS) $(HEADERS)
			printf "$(CURSIVE)$(GRAY) 	- Linking $(NAME)... $(RESET)\n"
			$(CXX) $(FLAGS) $^ -o $(NAME)
			printf "$(GREEN)    - Executable ready.\n$(RESET)"

clean	:
			$(RM) $(OBJS)
			printf "$(YELLOW)    - Object removed.$(RESET)\n"

fclean	:	clean
			$(RM) $(NAME)
			printf "$(YELLOW)    - Executable removed.$(RESET)\n"

re		: 	fclean all

.PHONY	: 	all clean fclean re

.SILENT	: 	clean fclean all $(NAME) $(OBJS)
