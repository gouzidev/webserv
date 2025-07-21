
#include "../../includes/webserv.hpp"
#include "../../includes/Debugger.hpp"


bool exists(map <string, string> &m, string key)
{
    return m.find(key) != m.end();
}

bool strHas(string str, string sub)
{
    return str.find(sub) != str.npos;
}


bool isValidPos(ssize_t pos)
{

    return (pos != string::npos && pos != -1);
}


bool strHas(string str, string sub, size_t pos = 0)
{
    return str.find(sub, pos) != str.npos;
}