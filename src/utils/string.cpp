#include "../../includes/webserv.hpp"

bool    strAllDigit(string s)
{
	size_t i = 0;
	while (s[i])
	{
		if (!isdigit(s[i]))
			return false;
		i++;
	}
	return true;
}

bool startsWith(string &str, string sub)
{
	if (str.size() < sub.size())
		return false;
	return (str.substr(0, sub.size()) == sub);
}

string trimSpaces(string &text)
{
        // const char *ws = "\t\n\r\f\v";
        string copy = text;
        copy.erase(copy.find_last_not_of(' ') + 1);
        copy.erase(0, copy.find_first_not_of(' '));
        return copy;
}

string trimSpaces(string text)
{
        // const char *ws = "\t\n\r\f\v";
        string copy = text;
        copy.erase(copy.find_last_not_of(' ') + 1);
        copy.erase(0, copy.find_first_not_of(' '));
        return copy;
}

string trimWSpaces(string &text)
{
        const char *ws = "\t\n\r\f\v ";
        string copy = text;
        copy.erase(copy.find_last_not_of(ws) + 1);
        copy.erase(0, copy.find_first_not_of(ws));
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

vector<string> split(string &str, char delim)
{
        size_t start;
        vector<string> v;
        size_t end = 0;
     
        while ((start = str.find_first_not_of(delim, end)) != string::npos) {
            end = str.find(delim, start);
            v.push_back(str.substr(start, end - start));
        }
        return v;
}

vector<string> splitNoSpace(string &str, char delim)
{
	size_t start;
	vector<string> v;
	size_t end = 0;
	
	while ((start = str.find_first_not_of(delim, end)) != string::npos) {
		end = str.find(delim, start);
		v.push_back(trimSpaces(str.substr(start, end - start)));
	}
	return v;
}

