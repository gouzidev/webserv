#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"
#include "../../../includes/Exceptions.hpp"

string readFromFile(string path) // for html files
{
    // (void)path;
    string content;
    // char path_[] = "/home/akoraich/ourwebserve/www/asma.html";
    std::ifstream file(path.c_str());
    if (file)
    {
        string line;
        // cout << "Reading file: " << path << endl;
        while (getline(file, line))
        {
            // cout << "line is " << line << endl;
            // cout << "last char is [" << line[line.size() - 1] << "]" << endl;
           content += line;
        }
        // cout << "content is " << content << endl;
        file.close();
        return content;
    }
    else
    {
        cout << "Error opening file: " << path << endl;
        return "";
    }
    return "";
}

// pair <string, string> parseData(string line)

// Replaces {{key}} placeholders in template file with values from data map
string dynamicRender(string path, map <string, string> &data)
{
    string content = "";
    std::ifstream file(path.c_str());
    if (file)
    {
        string line;
        while (getline(file, line))
        {
            size_t openBracketsPos = line.find("{{");
            while (openBracketsPos != string::npos)
            {
                size_t closeBracketsPos = line.find("}}");
                if (closeBracketsPos == string::npos)
                    throw ServerException("Unmatched opening brackets in line: " + line, 500);
                string key = line.substr(openBracketsPos + 2, closeBracketsPos - openBracketsPos - 2);
                if (!exists(data, key))
                    throw ServerException("Key not found in data map: " + key, 500);
                string val = data[key];
                if (val.empty())
                    val = "unkown";
                line.replace(openBracketsPos, closeBracketsPos - openBracketsPos + 2, val);
                openBracketsPos = line.find("{{");
            }
           content += line;
        }
        file.close();
    }
    else
        throw ServerException("Error opening file: " + path, 500);
    return content;
}


string getStatusMessage(unsigned short code) 
{
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 202: return "Accepted";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden"; 
        case 404: return "Not Found";
        case 409: return "Conflict";
        case 413: return "Request Entity Too Large";
        case 414: return "Request-URI Too Long";
        case 415: return "Unsupported Media Type";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        default: return "Unknown Error";
    }
}

string getErrorResponse(unsigned short errorCode, string body)
{
    string statusMsg = getStatusMessage(errorCode);
    string errorRes;
    errorRes += "HTTP/1.1 " + ushortToStr(errorCode) + " " + statusMsg + " \r\n";
    errorRes +=  "Content-Type: text/html\r\n";
    if (body != "")
    {
        errorRes +=  "Content-Length: " + ushortToStr(statusMsg.size()) + "\r\n\r\n";
        errorRes += statusMsg;
    }
    else
    {
        errorRes +=  "Content-Length: " + ushortToStr(body.size()) + "\r\n\r\n";
        errorRes += body;
    }
    return errorRes;
}

string getQuickResponse(short errCode, string fileStr)
{
    string res;
    res += "HTTP/1.1 " + ushortToStr(errCode) + " " + getStatusMessage(errCode) + " \r\n";
    res +=  "Content-Type: text/html\r\n";
    res +=  "Content-Length: " + ushortToStr(fileStr.size()) + "\r\n\r\n";
    res += fileStr;
    return res;
}

map <string , string> getErrorData(unsigned short errCode)
{
    map <string, string> errorData;
    switch (errCode) {
        case 400:
            errorData["errorText"] = "The server could not understand the request due to invalid syntax.";
            break;
        case 401:
            errorData["errorText"] = "You must authenticate yourself to get the requested response.";
            break;
        case 403:
            errorData["errorText"] = "You do not have permission to access the requested resource.";
            break;
        case 404:
            errorData["errorText"] = "The requested resource could not be found on this server.";
            break;
        case 500:
            errorData["errorText"] = "The server encountered an internal error and was unable to complete your request.";
            break;
        default:
            errorData["errorText"] = "An unexpected error occurred.";
    }
    errorData["errorCode"] = ushortToStr(errCode);
    errorData["errorTitle"] = getStatusMessage(errCode);
    return errorData;
}

void sendErrPageToClient(int clientfd, unsigned short errCode, ServerNode &servNode)
{
    string errorRes;
    string errorPageName;
    string errorPageStr;
    map <string , string> errorData;
    if (exists(servNode.errorNodes, errCode))
    {
        errorPageName = servNode.errorNodes[errCode]; 
        errorPageStr = readFromFile(errorPageName);
    }
    else
    {
        errorData = getErrorData(errCode);
        errorPageStr = dynamicRender(servNode.defaultErrorPage, errorData);
    }
    errorRes += "HTTP/1.1 " + ushortToStr(errCode) + " " + getStatusMessage(errCode) + " \r\n";
    errorRes +=  "Content-Type: text/html\r\n";
    errorRes +=  "Content-Length: " + ushortToStr(errorPageStr.size()) + "\r\n\r\n";
    errorRes += errorPageStr;
    cout << "error is {" << errorRes << "}" << endl;
    send(clientfd, errorRes.c_str(), errorRes.length(), 0);

}

