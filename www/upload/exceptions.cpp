#include "../../includes/webserv.hpp"
#include "../../includes/Exceptions.hpp"

WebServException::WebServException(const string msg, short errorCode)
{
    this->msg = msg;
    this->errorCode = errorCode;
}

short WebServException::getErrorCode()
{
    return errorCode;
}

WebServException::~WebServException() throw()
{

}

const char *WebServException::what() const throw ()
{
    return msg.c_str();
}

ServerException::ServerException(const string &msg, short errorCode) : WebServException("config error: " + msg, errorCode)
{
}

NetworkException::NetworkException(const string &msg, short errorCode) : WebServException("network error: " + msg, errorCode)
{
}

RequestException::RequestException(const string &msg, short errorCode, Request &req) : WebServException("request error: " + msg, errorCode), req(req)
{
}

Request &RequestException::getReq() const
{
    return req;
}

short RequestException::getErrorCode() const
{
    return errorCode;
}


















HeaderException::HeaderException(const string &msg, short errorCode, Request &request) : WebServException("header error: " + msg, errorCode), req(request)
{
}

Request &HeaderException::getReq() const
{
    return req;
}

short HeaderException::getErrorCode() const
{
    return errorCode;
}





















ConfigException::ConfigException(const string &msg, short errorCode) : WebServException("config error: " + msg, errorCode)
{
}
----------------------------268500534600953222758041
Content-Disposition: form-data; name="other"; filename="Makefile"
Content-Type: application/octet-stream

CC = c++

FLAGS = -Wall -Wextra -std=c++98 -g3# -fsanitize=address -fno-omit-frame-pointer
SRC_DIR = src
OBJ_DIR = obj

# Update paths based on your actual directory structure
FILES = $(SRC_DIR)/webserv.cpp \
        $(SRC_DIR)/config/parsing.cpp \
        $(SRC_DIR)/request/Request.cpp \
        $(SRC_DIR)/methods/get/get.cpp \
        $(SRC_DIR)/methods/post/post.cpp \
        $(SRC_DIR)/methods/post/upload.cpp \
        $(SRC_DIR)/methods/utils/utils.cpp \
        $(SRC_DIR)/utils/shortcuts.cpp \
        $(SRC_DIR)/utils/convert.cpp \
        $(SRC_DIR)/utils/path.cpp \
        $(SRC_DIR)/utils/string.cpp \
        $(SRC_DIR)/utils/User.cpp \
        $(SRC_DIR)/server/server.cpp \
        $(SRC_DIR)/server/Auth.cpp \
        $(SRC_DIR)/server/Session.cpp \
        $(SRC_DIR)/exceptions/exceptions.cpp


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