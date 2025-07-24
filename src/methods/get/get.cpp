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
    req.resp.fullResponse = req.resp.statusLine + "Content-Type: " + req.mimeType + "\r\n" + "Content-Length: " + contentLength + "\r\n\r\n" + fileContent;
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

void WebServ::dirList(string root, string location, Request req)
{
    vector<string> subDirs = getDirs(root);
    User loggedUser;
    string sessionKey;
    unsigned int userId;
    string dirlist = createDirList();
    dirlist += "<h1>DIRECTORY LISTING</h1><ul>";
    if (subDirs.empty() == true)
        dirlist += "<p>no sub-directories in this directory<br></p>";
    
    if (location == "/upload")
    {
        sessionKey = req.extractSessionId();
        if(!auth->isLoggedIn(sessionKey))
            throw ConfigException("Unauthorised", 401);
        Session session = auth->sessions.find(sessionKey)->second;
        loggedUser = session.getUser();
        userId = loggedUser.getId();
    }
    
    for (unsigned long i = 0; i < subDirs.size(); i++)
    {
        if (location == "/")
            dirlist += "<li><a href=\"" + subDirs[i] + "\">" + subDirs[i] + "</a></li>";
        else if (location == "/upload")
        {
            unsigned int fileId;
            string fileWithoutId = getOriginalFileName(req, subDirs[i], fileId);
            if (fileId != userId)
                continue;
            dirlist += "<li><a href=\"" + location + "/" + fileWithoutId + "\">" + fileWithoutId + "</a></li>";
        }
        else
            dirlist += "<li><a href=\"" + location + "/" + subDirs[i] + "\">" + subDirs[i] + "</a></li>";
    }

    dirlist += "</ul></body></html>";
    req.resp.setStatusLine("HTTP/1.1 200 OK\r\n");
    makeResponse(req, dirlist);
}

string WebServ::listUploadFiles(string root, Request req)
{
    vector<string> subDirs = getDirs(root);
    string dirlist;
    string sessionKey = req.extractSessionId();
    if (!auth->isLoggedIn(sessionKey))
        throw ConfigException("Unauthorised", 401);
    Session session = auth->sessions.find(sessionKey)->second;
    User loggedUser = session.getUser();
    unsigned int userId = loggedUser.getId();
    unsigned int fileId;
    cout << "user is " << loggedUser.getFirstName() << " id is " << userId << endl;
    for (unsigned long i = 0; i < subDirs.size(); i++)
    {
        string fileWithoutId = getOriginalFileName(req, subDirs[i], fileId);
        if (fileId != userId)
            continue;
        dirlist += "<div class=\"files-grid active\" id=\"gridView\">";
        dirlist +=  "<div class=\"file-card\" data-filepath=\"/upload/files/" + subDirs[i] + "\">";
        dirlist += fileWithoutId;
        dirlist +=  "<button type=\"submit\" class=\"delete-cross\"  data-filepath=\"/upload/files/" + subDirs[i] + "\" title=\"Delete file\">×</button>";
        dirlist += "</div></div>";
    }
    return dirlist;
}

string WebServ::uploadFile(string path, string root, Request req)
{
    map <string, string> data;
    string filesStr = listUploadFiles(root + "/files/", req);
    data["files"] = filesStr;
    string fileContent = dynamicRender(path, data);
    // fileContent += readFromFile("/home/akoraich/webserv/www/upload/files3.html");
    return fileContent;
}

bool WebServ::checkIndex(LocationNode node, Request req, string location)
{
    string fileContent;
    string fileName;

    for (unsigned long i = 0; i < node.index.size() ; i++)
    {
        if(node.root != "")
            fileName = node.root + "/" + node.index[i];
        if (location == "/upload")
            fileContent = uploadFile(node.root + "/" + node.index[i], node.root, req); // fiiiiix
        else
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

// void WebServ::handleGetFile(Request req)
// {
//     req.resp.setStatusLine("HTTP/1.1 200 OK\r\n");
//     string fileContent = readFromFile(req.fullResource);
//     makeResponse(req, fileContent);
// }

string sendBinaryResponse(const std::string& imagePath)
{
    std::ifstream file(imagePath.c_str(), std::ios::in | std::ios::binary);
    if (!file) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string imageData = ss.str();
    return imageData;
}

string WebServ::checkResource(string fullResource, string location)
{
    size_t i = 0;
    string newResource;
    while (fullResource[i])
    {
        // cout << "wieiiwiwiwiwiwiiwiw" << endl;
        if (fullResource[i] == '%')
            newResource += decodeHex(fullResource, i);
        else
            newResource += fullResource[i];
        i++;
    }
    return newResource;
}

string runCgi(Request req)
{
    int pipefd[2];
    pipe(pipefd);
    int id = fork();
    if (id == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        char *cmd[2];
        cmd[0] = strdup(req.fullResource.c_str());
        cmd[1] = NULL;
        execve(req.fullResource.c_str(), cmd, NULL);
        free(cmd[0]);
        exit(1);
    }
    close(pipefd[1]);
    string cgiOutput;
    char buffer[4096];
    ssize_t n;
    while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0)
       cgiOutput.append(buffer, n);
    close(pipefd[0]);
    waitpid(id, NULL, 0);
    cout << "OUTPUT IS FOR CGI " << cgiOutput << endl;
    return (cgiOutput);
}

void WebServ::handleGetFile(Request req, map<string, string> &data)
{
    string fileContent;
    req.resp.setStatusLine("HTTP/1.0 200 OK\r\n");
    if (req.mimeType == "text/html")
    {
        if (data.empty())
            fileContent = readFromFile(req.fullResource);
        else
            fileContent = dynamicRender(req.fullResource, data);
        makeResponse(req, fileContent);
    }
    else if (req.mimeType == "cgi")
    {
        req.mimeType = "text";
        fileContent = runCgi(req);
        makeResponse(req, fileContent);
    }
    else if (req.mimeType == "not supported")
    {
        req.mimeType = "text";
        fileContent = "this file type is not supported";
        makeResponse(req, fileContent);
    }
    else
    {
        cout << "Serving binary file: " << req.fullResource << endl;
    
        int filefd = open(req.fullResource.c_str(), O_RDONLY);
        if (filefd == -1) {
            cout << "Failed to open file: " << req.fullResource << endl;
            sendErrPageToClient(req.cfd, 404, req.serv);
            return;
        }
    
        // Get file size
        struct stat fileStat;
        if (fstat(filefd, &fileStat) == -1) {
            close(filefd);
            sendErrPageToClient(req.cfd, 500, req.serv);
            return;
        }

        // Send headers
        stringstream contentLength;
        contentLength << fileStat.st_size;
        
        string responseHeader = "HTTP/1.1 200 OK\r\n";
        responseHeader += "Content-Type: " + req.mimeType + "\r\n";
        responseHeader += "Content-Length: " + contentLength.str() + "\r\n\r\n";
        
        ssize_t headerSent = send(req.cfd, responseHeader.c_str(), responseHeader.size(), 0);
        if (headerSent <= 0)
        {
            cout << "Failed to send headers" << endl;
            close(filefd);
            return;
        }
        char buff[BUFFSIZE];
        ssize_t bytesRead;
        ssize_t totalSent = 0;
        bytesRead = 1;
        do
        {
            bytesRead = read(filefd, buff, BUFFSIZE);
            ssize_t bytesSent = 0;
            ssize_t totalToSend = bytesRead;
            
            while (bytesSent < totalToSend)
            {
                ssize_t sent = send(req.cfd, buff + bytesSent, totalToSend - bytesSent, 0);
                if (sent <= 0)
                {
                    cout << "Send failed after " << totalSent << " bytes" << endl;
                    close(filefd);
                    return;
                }
                bytesSent += sent;
                totalSent += sent;
            }
        }
        while (bytesRead > 0);
        close(filefd);
        cout << "File sent successfully: " << totalSent << " bytes" << endl;
    }
}

bool isDirectory(string& path)
{
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
    if (location == target)
        return root;
    string path = target.substr(location.size());
    return root + path;
}

void WebServ::requestChecks(Request &req, ServerNode &serv, string &location, LocationNode &node)
{
    string target = req.getResource();
    cout << "target is " << target << endl;
    location = getLocation(req, serv);
    if (location == "")
        throw ConfigException("forbidden request", 404);
    node = serv.locationDict.find(location)->second;
    if (!exists(node.methods, req.getReqType()))
        throw ConfigException(getStatusMessage(405), 405);
    req.fullResource = getFullResource(node.root, location, target);
}

void WebServ::handleGetUpload(Request req, LocationNode node, User loggedUser, string location)
{
    unsigned int userId = loggedUser.getId();
    map <string, string> data = loggedUser.getKeyValData();
    int i = req.fullResource.size();
    while (i > 0)
    {
        if (req.fullResource[i - 1] == '/')
           break;
        i--;
    }
    string fileName = req.fullResource.substr(i);
    string uploadResource = node.root + "/" + getFileNameWithUserId(req, userId ,fileName); //will need updates
    if (isDirectory(req.fullResource) == true)
    {
        if (node.index.empty() == true || checkIndex(node, req, location) == 1)
        {
            if(node.autoIndex == true)
                dirList(node.root, location, req);
            else
                throw ConfigException("forbidden request", 404);
        }
    }
    else if (isRegularFile(uploadResource) == true)
    {
        req.fullResource = uploadResource;
        cout << "should be fileeee" << endl;
        getMimeType(req);
        handleGetFile(req, data);
    }
    else
        throw ConfigException("anaaa", 404);
}

void WebServ::getMethode(Request &req, ServerNode &serv)
{
    string sessionKey;
    map <string, string> data ;
    string location;
    LocationNode node;
    User loggedUser;
    unsigned int userId;
    try
    {
        requestChecks(req, serv, location, node);
        //if location  is upload : 1. same as is protected 2. use encoding function (check if the user id is the same as the one in the file)
        req.fullResource = checkResource(req.fullResource, location);
        if (node.isProtected)
        {
            sessionKey = req.extractSessionId();
            if (!auth->isLoggedIn(sessionKey))
                throw ConfigException("Unauthorised", 401);
            Session session = auth->sessions.find(sessionKey)->second;
            loggedUser = session.getUser();
            data = loggedUser.getKeyValData();
            cout << "user id " << userId << endl;
        }
        if (location == "/upload")
            handleGetUpload(req, node, loggedUser, location);
        else if (isDirectory(req.fullResource) == true)
        {
            if (node.index.empty() == true || checkIndex(node, req, location) == 1)
            {
                if(node.autoIndex == true)
                dirList(node.root, location, req);
                else
                throw ConfigException("forbidden request", 404);
            }
        }
        else if (isRegularFile(req.fullResource) == true)
        {
            cout << "should be fileeee" << endl;
            getMimeType(req);
            handleGetFile(req, data);
        }
        else
        {
            throw ConfigException("Not Found", 404);
        }
    }
    catch(ConfigException& e)
    {
        sendErrPageToClient(req.cfd, e.getErrorCode(), req.serv);
        std::cerr << e.what() << '\n';
    }
}