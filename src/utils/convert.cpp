#include "../../includes/webserv.hpp"
#include "../../includes/Exceptions.hpp"

// Convert port to string
string ushortToStr(unsigned short port)
{
    stringstream ss;
    ss << port;
    string portStr = ss.str();
    return portStr;
}

long stringToHexLong(std::string str, Request &req)
{
    long res = 0;
    std::string hexChars = "0123456789abcdef";
    for (size_t i = 0; i < str.size(); i++)
    {
        size_t pos = hexChars.find(str[i]);
        if (pos == std::string::npos)
            throw RequestException("invalid hex string", 400, req);
        res *= 16;
        long temp = (long) pos;
        res += temp;
    }
    return res;
}
