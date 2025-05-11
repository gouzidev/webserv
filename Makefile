CC = c++

FLAGS = -Wall -Wextra -Werror -std=c++98 -g3

SRC_DIR = src
OBJ_DIR = obj

FILES = $(SRC_DIR)/webserv.cpp \
		$(SRC_DIR)/parsing.cpp \
		$(SRC_DIR)/string.cpp \
		$(SRC_DIR)/Request.cpp \
		$(SRC_DIR)/server.cpp

OBJ = $(FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

NAME = webserv

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

all: $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)

re: clean all

fclean: clean
	rm -rf $(NAME)