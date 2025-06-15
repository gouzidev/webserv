
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