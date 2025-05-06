CC = c++

FLAGS = -Wall -Wextra -Werror -std=c++98

FILES = src/webserv.cpp src/parsing.cpp src/string.cpp

OBJ = $(FILES:.cpp=.o)

NAME = webserv

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

all: $(NAME)

%.o: %.cpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)

re: clean all

fclean: clean
	rm -rf $(NAME)