#include "../includes/webserv.hpp"

void Request::setStartLine(string line)
{
    start_line = split(line, ' ');
}

vector <string> Request::getStartLine()
{
    return start_line;
}

string Request::getTargetResource()
{
    return start_line[1];
}

void Request::setHeaders(string line)//needs checking for headers syntax
{
    vector <string> header_line = split(line, ':');
    pair <string, string> p = make_pair(header_line[0], header_line[1]);
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
        return 1;
    if (start_line[0].find("GET") != string::npos)
    {
        cout << "we got get" << endl;
        req_type = GET;
    }
    else if (start_line[0].find("POST") != string::npos)
        req_type = POST;
    else if (start_line[0].find("DELETE") != string::npos)
        req_type = DELETE;
    else
        return 1;
    if (start_line[1].find("/") != 0)
    {
        cout << "invalid path" << endl;
        return 1;
    }
    if (start_line[2].find("HTTP/1.1") == string::npos)
    {
        cerr << "[ " << start_line[2] << " ] " << "invalid http ver" << endl;
        return 1;
    }
    return 0;
}