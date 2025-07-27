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
















HttpException::HttpException(short errorCode, Client &client) : errorCode(errorCode), client(client)
{
}

short HttpException::getErrorCode() const
{
    return errorCode;
}

Client &HttpException::getClient() const
{
    return client;
}