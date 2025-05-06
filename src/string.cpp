#include "../includes/webserv.hpp"

string trimSpaces(string &text)
{
        // const char *ws = "\t\n\r\f\v";
        string copy = text;
        copy.erase(copy.find_last_not_of(' ') + 1);
        copy.erase(0, copy.find_first_not_of(' '));
        return copy;
}

bool    isStrEq(string a, string b)
{
        if (a.size() >= b.size())
        {
                return a == b;
        }
        return false;
}