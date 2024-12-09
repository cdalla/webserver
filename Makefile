# **************************************************************************** #
#                                                                              #
#                                                         ::::::::             #
#    Makefile                                           :+:    :+:             #
#                                                      +:+                     #
#    By: nsterk <nsterk@student.codam.nl>             +#+                      #
#                                                    +#+                       #
#    Created: 2022/04/13 14:42:55 by nsterk        #+#    #+#                  #
#    Updated: 2024/11/16 14:55:53 by cdalla-s      ########   odam.nl          #
#                                                                              #
# **************************************************************************** #

NAME		:=	webserv

INC_DIR		:=	inc
SRCS_DIR	:=	srcs
OBJ_DIR		:=	objs
VPATH		:=	$(subst $(space),:,$(shell find srcs -type d))

SRCS		=	main.cpp webserver.cpp webserver_epoll.cpp utils.cpp client.cpp server.cpp responseHandler.cpp requestParser.cpp serverParser.cpp locationParser.cpp config.cpp baseParser.cpp file.cpp cgi.cpp
OBJS		=	$(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

CC			:=	c++
FLAGS		:= -std=c++11 -g -I$(INC_DIR) -fsanitize=address

all:		$(NAME)

$(NAME):	$(OBJS)
	$(CC) $(OBJS) $(FLAGS) -o $(NAME)

$(OBJ_DIR)/%.o: $(notdir %.cpp)
	@ mkdir -p $(OBJ_DIR)
	$(CC) $(FLAGS) -c $< -o $@

run:	all
	@ ./$(NAME)

clean:
	@ rm -rf $(OBJ_DIR)

fclean: clean
	@ rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re
