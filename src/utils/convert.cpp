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

string ulongToStr(unsigned long num)
{
    stringstream ss;
    ss << num;
    string numStr = ss.str();
    return numStr;
}

long WebServ::stringToHexLong(std::string str, Client &client)
{
    long res = 0;
    std::string hexChars = "0123456789abcdef";
    for (size_t i = 0; i < str.size(); i++)
    {
        size_t pos = hexChars.find(str[i]);
        if (pos == std::string::npos)
            throw HttpException(400, client);
        res *= 16;
        long temp = (long) pos;
        res += temp;
    }
    return res;
}
