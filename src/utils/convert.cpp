#include "../../includes/webserv.hpp"

// Convert port to string
string ushortToStr(unsigned short port)
{
    stringstream ss;
    ss << port;
    string portStr = ss.str();
    return portStr;
}

