#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"
#include "../../../includes/Exceptions.hpp"
#include <sstream>
// bool isPathValid(string path)
// {
    
//     return true;
// }

void Response::setStatusLine(string sttsLine)
{
    statusLine = sttsLine;
}

void makeResponse(Request req, string fileContent)
{
    std::stringstream ss;
    ss << fileContent.length();
    string contentLength = ss.str();
    // "\r\n";
    // "Hello, World!";
    // string File = readFromFile(indexPath);

    // const char *testResponse =
    // "HTTP/1.1 200 OK\r\n"
    // "Content-Type: text/plain\r\n"
    // "Content-Length: " + strlen(File.c_str()) + "\r\n";
    req.resp.fullResponse = req.resp.statusLine + "Content-Type: text/html\r\n" + "Content-Length: " + contentLength + "\r\n\r\n" + fileContent;
    // cout << "response is [ " << req.resp.fullResponse << " ]" << endl;
    send(req.cfd, req.resp.fullResponse.c_str(), req.resp.fullResponse.size(), 0);
}

string createDirList()
{
    return "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Document</title></head><body>";
}

vector<string> getDirs(string mainDir)
{
    std::vector<std::string> dirs;
    DIR* dir = opendir(mainDir.c_str());
    if (!dir) return dirs;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        std::string fullPath = mainDir + "/" + entry->d_name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0) {
            dirs.push_back(entry->d_name);
        }
    }
    closedir(dir);
    return dirs;
}

void dirList(string root, string location, Request req)
{
    vector<string> subDirs = getDirs(root);
    string dirlist = createDirList();
    dirlist += "<h1>DIRECTORY LISTING</h1><ul>";
    if (subDirs.empty() == true)
        dirlist += "<p>no sub-directories in this directory<br></p>";
    for (int i = 0; i < subDirs.size(); i++)
    {
        if (location == "/")
            dirlist += "<li><a href=\"" + subDirs[i] + "\">" + subDirs[i] + "</a></li>";
        else
            dirlist += "<li><a href=\"" + location + "/" + subDirs[i] + "\">" + subDirs[i] + "</a></li>";
    }
    dirlist += "</ul></body></html>";
    req.resp.setStatusLine("HTTP/1.1 200 OK\r\n");
    makeResponse(req, dirlist);
}

// void Response::setHeaders(string header)
// {
// }

bool checkIndex(LocationNode node, Request req)
{
    string fileContent;
    string fileName;
    for (unsigned long i = 0; i < node.index.size() ; i++)
    {
        if(node.root != "")
            fileName = node.root + "/" + node.index[i];
        // else
        //      ;
        fileContent = readFromFile(fileName);
        if (fileContent != "")
        {
            cout << "hello" << endl;
            req.resp.setStatusLine("HTTP/1.1 200 OK\r\n");
            makeResponse(req, fileContent);
            return 0;
        }
    }
    throw ConfigException("couldnt find the index", 500);
}

// string getFilePath(Request req)
// {
//     string path;
//     while(req.resource.)
// }

void WebServ::handleGetFile(Request req, map<string, string> &data)
{
    
    req.resp.setStatusLine("HTTP/1.1 200 OK\r\n");
    string fileContent = dynamicRender(req.fullResource, data);
    makeResponse(req, fileContent);
}

bool isDirectory(string& path){
    struct stat st;
    if (stat(path.c_str(), &st) == 0)
        return S_ISDIR(st.st_mode);
    return false;
}

bool isRegularFile(string& path)
{
    struct stat st;
    if (stat(path.c_str(), &st) == 0)
        return S_ISREG(st.st_mode);
    return false;
}

string getFullResource(string root, string location, string target)
{
    if (location == target || location == "/")
        return root;
    string path = target.substr(location.size());
    return root + path;
}

void WebServ::getMethode(Request req, ServerNode serv)
{
    string sessionKey;
    string target = req.getResource();
    string location = getLocation(req, serv);
    // cout << "restOfLocation is [ " << restOfLocation << " ]" << endl;
    if (location == "")
    {
        string errorRes  = getErrorResponse(404, "");
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
    }
    LocationNode node = serv.locationDict.find(location)->second;
    // Debugger::printLocationNode(node);
    if (!exists(node.methods, string("GET")))
    {
        string errorRes  = getErrorResponse(405, "");
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
    }
    req.fullResource = getFullResource(node.root, location, target);
    try
    {
        // if (node.isProtected)
        // {
        //     string sessionKey = req.extractSessionId();
        //     if (!auth->isLoggedIn(sessionKey))
        //     {
        //         sendErrPageToClient(req.cfd, 401, serv);
        //         return ;
        //     }
        // } /home/akoraich/webserv/www/login/login.html

        // string resPath = node.root + "/" + req.resource;
        if (isDirectory(req.fullResource) == true)
        {
            if (node.index.empty() == true || checkIndex(node, req) == 1)
            {
                if(node.autoIndex == true)
                    dirList(node.root, location, req);
                // else error 403/404
            }
        }
        else if (isRegularFile(req.fullResource) == true)
        {
            if (node.isProtected)
            {
                sessionKey = req.extractSessionId();
                if (!auth->isLoggedIn(sessionKey))
                {
                    sendErrPageToClient(req.cfd, 401, serv);
                    return ;
                }
            }
            
            Session session = auth->sessions.find(sessionKey)->second;
            User loggedUser = session.getUser();
            map <string, string> data = loggedUser.getKeyValData();
            // data['email'] = loggedUser.getEmail();
            cout << "is file " << endl;
            handleGetFile(req, data);
        }
        else
            throw ConfigException("forbidden request", 404);
        // }
        // else
        //     handleGetFile(req);
        // cout << "HOW!!!!!" << endl;
    }
    catch(ConfigException& e)
    {
        sendErrPageToClient(req.cfd, e.getErrorCode(), req.serv);
        std::cerr << e.what() << '\n';
    }
}