
#include "../../includes/webserv.hpp"
#include "../../includes/Debugger.hpp"


bool exists(map <string, string> &m, string key)
{
    return m.find(key) != m.end();
}

bool existsAndIs(map <string, string> &m, string key, string val)
{
    map <string, string>::iterator keyPos = m.find(key);
    if (keyPos != m.end())
    {
        return m.at(key) == val; 
    }
    return false;
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