#include "../includes/webserv.hpp"

void Request::setStartLine(string line)
{
    start_line = split(line, ' ');
    string location = start_line[1];
    size_t posOfSep = location.find_first_of('?');
    if (posOfSep != location.npos)
    {
        resource = location.substr(0, posOfSep);
        string queryStr = location.substr(posOfSep + 1);
        if (queryStr.size() < 1)
            return ;
        fillQuery(queryStr);
    }
    else
        resource = location;
}

void Request::fillQuery(string queryStr)
{
    size_t posOfSep;
    string key;
    string val;
    vector <string> queriesStr = split(queryStr, '&');
    for (size_t i = 0; i < queriesStr.size(); i++)
    {
        posOfSep = queriesStr[i].find('=');
        if (posOfSep != queriesStr[i].npos)
        {
            key = queriesStr[i].substr(0, posOfSep);
            val = queriesStr[i].substr(posOfSep + 1);
        }
        else
        {
            key = queriesStr[i];
            val = "";
        }
        queryParams[key] = val;
        cout << key << " -> " << val << endl;
    }
}

vector <string> Request::getStartLine()
{
    return start_line;
}

string Request::getResource()
{
    return resource;
}

void Request::setHeaders(string line) //needs checking for headers syntax
{
    size_t sepPos = line.find_first_of(':');
    if (sepPos == line.npos)
        return ;
    string key;
    string val;
    key = line.substr(0, sepPos);
    val = line.substr(sepPos + 1);

    // lower case the header key (the key is case insensitive):
    transform(key.begin(), key.end(), key.begin(), ::tolower);
    pair <string, string> p = make_pair(key, val);
    headers.insert(p);
}

void Request::setBody(string line)
{
    body.push_back(line);
}

int Request::getReqType()
{
    return req_type;
}

int Request::isStartLineValid()
{
    if (start_line.size() != 3)
        return ERROR;
    if (start_line[0] == "GET")
    {
        cout << "we got get" << endl;
        req_type = GET;
    }
    else if (start_line[0] == "POST")
        req_type = POST;
    else if (start_line[0] == "DELETE")
        req_type = DELETE;
    else
        return ERROR;
    if (start_line[1].find("/") != 0)
    {
        cout << "invalid path" << endl;
        return ERROR;
    }
    if (start_line[2].find("HTTP/1.1") == string::npos)
    {
        cerr << "[ " << start_line[2] << " ] " << "invalid http ver" << endl;
        return ERROR;
    }
    return 0;
}