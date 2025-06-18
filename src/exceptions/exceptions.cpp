#include "../../includes/webserv.hpp"
#include "../../includes/Exceptions.hpp"

WebServException::WebServException(const string &msg, short errorCode)
{
    this->msg = msg;
    this->errorCode = errorCode;
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

RequestException::RequestException(const string &msg, short errorCode, ServerNode &serv) : serv(serv), WebServException("request error: " + msg, errorCode)
{
}

HeaderException::HeaderException(const string &msg, short errorCode, ServerNode &serv) : serv(serv), WebServException("header error: " + msg, errorCode)
{
}

ConfigException::ConfigException(const string &msg, short errorCode) : WebServException("config error: " + msg, errorCode)
{
}