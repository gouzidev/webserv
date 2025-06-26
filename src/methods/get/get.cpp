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
        if (stat(fullPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            dirs.push_back(entry->d_name);
        }
    }
    closedir(dir);
    return dirs;
}

void dirList(string root, Request req)
{
    vector<string> subDirs = getDirs(root);
    string dirlist = createDirList();
    dirlist += "<h1>DIRECTORY LISTING</h1><ul>";
    for (int i = 0; i < subDirs.size(); i++)
        dirlist += "<li><a href=\"" + subDirs[i] + "\">" + subDirs[i] + "</a></li>";
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

void WebServ::handleGetFile(Request req)
{
    vector<string> subdirs = getDirs(req.serv.root);
    DIR* dir = opendir(req.serv.root.c_str());
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.') continue;
        std::string fullPath = req.serv.root + "/" + entry->d_name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0)
        {
            string resource = req.resource.substr(1);
            // cout << "full path isssss [" << fullPath << "]" << endl;
            // cout << "entry name isssss [" << entry->d_name << "]" << endl;
            // cout << "resource isssss [" << resource + ".html" << "]" << endl;
            if (S_ISDIR(st.st_mode))
            {
                // cout << "wiwiwiwiwi" << endl;
            }
            else if (S_ISREG(st.st_mode) && entry->d_name == resource + ".html")
            {
                // cout << "ouiiiiiiiiiiiiiiiiiiiiiiii" << endl;
                req.resp.setStatusLine("HTTP/1.1 200 OK\r\n");
                
                // if file needs login ->  dynamic render dashboard or profile
                // else just static render it.
                makeResponse(req, readFromFile(fullPath));
            }

        }
    }
        // if (stat(fullPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        //     dirs.push_back(entry->d_name);
        // }
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

void WebServ::getMethode(Request req, ServerNode serv)
{
    string target = req.getResource();
    cout << "target is [ " << target << " ]" << endl;
    string location = getLocation(target, serv);
    cout << "location is [ " << location << " ]" << endl;
    if (!exists(serv.locationDict, location))
    {
        cout << "location not found in server node" << endl;
        string errorRes  = getErrorResponse(404, "");
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
    }
    LocationNode node = serv.locationDict.find(location)->second;
    
    if (!exists(node.methods, string("GET")))
    {
        cout << "method not allowed for this location" << endl;
        string errorRes  = getErrorResponse(405, "");
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
    }
    try
    {

        if (node.isProtected)
        {
            string sessionKey = req.extractSessionId();
            if (!auth->isLoggedIn(sessionKey))
            {
                sendErrPageToClient(req.cfd, 401, serv);
                return ;
            }
        }


        string resPath = node.root + "/" + req.resource; // signup
        cout << "resPath is [ " << resPath << " ]" << endl;
        if (isDirectory(resPath) == true)
        {
            cout << "d5lat" << endl;
            if (node.index.empty() == true || checkIndex(node, req) == 1)
            {
                if(node.autoIndex == true)
                    dirList(node.root, req);
                // else error 403/404
            }
        }
        else
            handleGetFile(req);
        // cout << "HOW!!!!!" << endl;
    }
    catch(ConfigException& e)
    {
        sendErrPageToClient(req.cfd, req.error, req.serv);
        std::cerr << e.what() << '\n';
    }
}
    // cout << "location is [ " << location << " ]" << endl;
    // const char *testResponse =
    // "HTTP/1.1 200 OK\r\n"
    // "Content-Type: text/plain\r\n"
    // "Content-Length: 13\r\n"
    // "\r\n";
    // // "Hello, World!";
    // string requestedFile = readFromFile(req.resource);
    // Response resp;
    // resp.fullResponse = testResponse + requestedFile;
    // cout << "response is [ " << resp.fullResponse << " ]" << endl;
    // send(req.cfd, resp.fullResponse.c_str(), resp.fullResponse.size(), 0);
    // // cerr << testResponse;
// }