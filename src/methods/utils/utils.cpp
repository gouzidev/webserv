#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"

string readFromFile(string path) // for html files
{
    // (void)path;
    string content;
    // char path_[] = "/home/akoraich/ourwebserve/www/asma.html";
    std::ifstream file(path.c_str());
    if (file)
    {
        string line;
        cout << "Reading file: " << path << endl;
        while (getline(file, line))
        {
            // cout << "line is " << line << endl;
            // cout << "last char is [" << line[line.size() - 1] << "]" << endl;
           content += line + "\r\n";
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

string getStatusMessage(unsigned short code) 
{
    switch (code) {
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden"; 
        case 404: return "Not Found";
        case 409: return "Conflict";
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

void sendErrToClient(int clientfd, unsigned short errCode, ServerNode &servNode)
{
    string errorRes;
    const char *generalErrorResponse =
        "HTTP/1.1 500 INTERNAL SERVER ERROR\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Server Error";
    if (exists(servNode.errorNodes, errCode)) 
    {
        cout << "Error code found in error nodes, sending specific error response" << endl;
        string errorFileStr = servNode.errorNodes.find(errCode)->second;
        cout << errorFileStr << endl;
        if (!validPath(errorFileStr) || !checkFile(errorFileStr, O_RDONLY))
        {
            send(clientfd, generalErrorResponse, strlen(generalErrorResponse), 0);
        }
        else
        {
            ifstream errorFile;
            errorFile.open(errorFileStr.c_str());
            if (errorFile.fail())
            {
                cerr << "Error happened opening the file" << endl;
                send(clientfd, generalErrorResponse, strlen(generalErrorResponse), 0);
                return ;
            }
            string htmlErrFileStr, line;
            while (getline(errorFile, line))
            {
                htmlErrFileStr += line + "\r\n";
            }
            errorRes += "HTTP/1.1 " + ushortToStr(errCode) + " " + getStatusMessage(errCode) + " \r\n";
            errorRes +=  "Content-Type: text/html\r\n";
            errorRes +=  "Content-Length: " + ushortToStr(htmlErrFileStr.size()) + "\r\n\r\n";
            errorRes += htmlErrFileStr;
            send(clientfd, errorRes.c_str(), errorRes.length(), 0);
        }
    }
    else
    {
        cout << "Error code not found in error nodes, sending general error response" << endl;
        cout << "sending -> " << generalErrorResponse << endl;
        send(clientfd, generalErrorResponse, strlen(generalErrorResponse), 0);
    }
}

