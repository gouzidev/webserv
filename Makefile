CC = c++

FLAGS = -Wall -Wextra  -std=c++98 -g3

SRC_DIR = src
OBJ_DIR = obj

# Update paths based on your actual directory structure
FILES = $(SRC_DIR)/webserv.cpp \
        $(SRC_DIR)/config/parsing.cpp \
        $(SRC_DIR)/request/Request.cpp \
        $(SRC_DIR)/methods/get/get.cpp \
        $(SRC_DIR)/methods/post/post.cpp \
        $(SRC_DIR)/methods/utils/utils.cpp \
        $(SRC_DIR)/utils/shortcuts.cpp \
        $(SRC_DIR)/utils/convert.cpp \
        $(SRC_DIR)/utils/path.cpp \
        $(SRC_DIR)/utils/string.cpp \
        $(SRC_DIR)/utils/User.cpp \
        $(SRC_DIR)/server/server.cpp


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