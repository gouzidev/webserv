#include "../../includes/webserv.hpp"
#include "../../includes/Debugger.hpp"
#include "../../includes/Exceptions.hpp"

Request::Request(ServerNode &serv) : serv(serv)
{
    contentLen = 0;
}

void Request::setStartLine(string line)
{
    startLine = split(line, ' ');
    string location = startLine[1];
    size_t posOfSep = location.find_first_of('?');
    if (posOfSep != location.npos)
    {
        resource = location.substr(0, posOfSep);
        string queryStr = location.substr(posOfSep + 1);
        if (queryStr.size() < 1)
            throw RequestException("bad location block in start line", 400, *this);
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

vector <string> & Request::getStartLine()
{
    return startLine;
}

map  <string ,string> & Request::getHeaders()
{
    return headers;
}

string removeTrailingCR(string str)
{
    if (!str.empty() && str[str.size() - 1] == '\r')
        str = str.substr(0, str.size() - 1);
    return str;
}

string & Request::getResource()
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

    key = trimWSpaces(key);
    val = trimWSpaces(val);
    // lower case the header key (the key is case insensitive):
    transform(key.begin(), key.end(), key.begin(), ::tolower); // Field names are case-insensitive rfc 4.2 msg headers
    pair <string, string> p = make_pair(key, val);
    headers.insert(p);

    if (key == "content-length")
    {
        contentLen = extractContentLen(*this, serv);
    }
}

// for the body parsing : If a message is received with both a
    //  Transfer-Encoding header field and a Content-Length header field,
    //  the latter MUST be ignored

void Request::setBody(string line) // must handle checked body if its in the headers, chunked according to the rfc is sending the number of bytes then sebding the bytes, when sending 0, indicates that its done :  // Send data as it's generated: send("5\r\n");        // Chunk size: 5 bytes,   send("Hello\r\n");    // Data: "Hello",   send("7\r\n");        // Chunk size: 7 bytes,    send(" World!\r\n");  // Data: " World!",    send("0\r\n\r\n");    // End chunks
{
    body += line;
}

int Request::getReqType()
{
    return reqType;
}

string getLocation(string resource, ServerNode &servNode)
{
    string location = resource;
    size_t i = 1;
    for (; i < resource.size(); i++)
    {
        if (resource[i] == '/')
            break;
    }
    location = resource.substr(0, i);
    if (!exists(servNode.locationDict, location))
    {
        string defaultLocation = "/";
        if (exists(servNode.locationDict, defaultLocation))
            return defaultLocation;
    }
    return location;
}

// the resource will be starting with a slash
// this function will return the location of the resource in the server node, ex : 
// GET /auth/login HTTP/1.1
// location -> /auth

int hexCharToInt(char c)
{
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;   // A-F
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;   // a-f
    else
        return c - '0';        // 0-9
}

char decodeHex(string &str, size_t &idx)
{
    char decoded = '%';
    if (idx + 2 < str.size())
    {
        int digit1 = hexCharToInt(str[idx + 1]);
        int digit2 = hexCharToInt(str[idx + 2]);
        if (digit1 >= 0 && digit2 >= 0) {
            int asciiValue = digit1 * 16 + digit2;
            decoded = (char)asciiValue;
            idx += 2;
        } else {
            decoded = str[idx]; // Invalid hex, keep the %
        }
    }
    else
        decoded = str[idx];
    return decoded;
}

void WebServ::urlFormParser(string str, map <string, string> &queryParms)
{
    string temp = "";
    string key = "";
    size_t i = 0;
    while (i < str.size())
    {
        if (str[i] == '=')
        {
            key = temp;
            temp = "";
        }
        else if (str[i] == '&')
        {
            queryParms[key] = temp;
            cout << "key: " << key << " value: " << temp << endl;
            key = "";
            temp = "";
        }
        else if (str[i] == '%')
        {
            temp += decodeHex(str, i);
        }
        else
        {
            temp += str[i];
        }
        i++;
    }
    if (!key.empty())
        queryParms[key] = temp;
}

int Request::isStartLineValid()
{
    if (startLine.size() != 3)
        throw RequestException("start line syntax is incorrect", 400, *this);
    transform(startLine[0].begin(), startLine[0].end(), startLine[0].begin(), ::toupper); // rfc 5.1.1  The method is case-sensitive.
    if (startLine[0] == "GET")
        reqType = GET;
    else if (startLine[0] == "POST")
        reqType = POST;
    else if (startLine[0] == "DELETE")
        reqType = DELETE;
    else
        throw RequestException("start line is errornous, unidentified request type", 400, *this);
    if (startLine[1].find("/") != 0)
    {
        cout << "invalid path" << endl;
        throw RequestException("start line is errornous, invalid path", 400, *this);
    }
    if (startLine[2].find("HTTP/1.1") == string::npos)
    {
        throw RequestException("start line is errornous, missing HTTP/1.1", 400, *this);
    }
    return 0;
}