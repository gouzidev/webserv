
#include "../includes/webserv.hpp"


bool exists(map <string, string> &m, string key)
{
    return m.find(key) == m.end();
}

