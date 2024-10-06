NAME := webserver

CC := c++

FLAGS := -Wall -Werror -Wextra

HEADER :=	webserver.hpp\
			server.hpp\
			client.hpp\
			socket.hpp\
			request.hpp\
			response.hpp

SRC := 	main.cpp\
		webserver.cpp\
		webserver_epoll.cpp\
		server.cpp\
		client.cpp\
		utils.cpp\
		request.cpp\
		response.cpp

OBJ =	$(SRC:%.cpp=obj/%.o)
		
all: $(NAME)
	
$(NAME): $(OBJ) $(HEADER)
	@$(CC) $(FLAGS) $(OBJ) -o $@

obj/%.o:%.cpp $(HEADER)
	@mkdir -p obj
	$(CC) -c $(FLAGS) $< -o $@

clean:
	rm -r obj

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all, clean, fclean, re