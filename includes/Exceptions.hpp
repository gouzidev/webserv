#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <exception>
#include "webserv.hpp"

class WebServException: public exception
{
    public:
        WebServException(const string &msg, short errorCode);
        virtual const char *what() const throw ();
        virtual ~WebServException() throw(); 
    protected:
        short errorCode;
        string msg;
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
        ServerNode &serv;
    public :
        RequestException(const string &msg, short errorCode, ServerNode &serv);
};

class HeaderException:  public WebServException
{
    private:
        ServerNode &serv;
    public :
        HeaderException(const string &msg, short errorCode, ServerNode &serv);
};

class ServerException:  WebServException
{
    public :
        ServerException(const string &msg, short errorCode);
};




#endif
