#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <exception>
#include "webserv.hpp"

class HttpException: public exception
{
    public:
        HttpException(short errorCode);
        short getErrorCode() const;
    protected:
        short errorCode;
};


class WebServException: public exception
{
    public:
        WebServException(const string msg, short errorCode);
        virtual const char *what() const throw ();
        virtual ~WebServException() throw(); 
        short getErrorCode();
    protected:
        string msg;
        short errorCode;
};

class NetworkException:  public WebServException
{
    public :
        NetworkException(const string &msg, short errorCode);
};

class ConfigException:  public WebServException
{
    public :
        ConfigException(const string &msg, short errorCode);
};

class RequestException:  public WebServException
{
    private:
        Request &req;
    public :
        Request &getReq() const;
        short getErrorCode() const;
        RequestException(const string &msg, short errorCode, Request &req);
};


















class HeaderException:  public WebServException
{
    private:
        Request &req;
    public :
        Request &getReq() const;
        short getErrorCode() const;
        HeaderException(const string &msg, short errorCode, Request &req);
};














class ServerException:  public WebServException
{
    public :
        ServerException(const string &msg, short errorCode);
};




#endif
