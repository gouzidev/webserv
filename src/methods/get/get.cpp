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

void makeResponse(Client &client, string fileContent)
{
    std::stringstream ss;
    ss << fileContent.length();
    string contentLength = ss.str();
    // "\r\n";
    // "Hello, World!";
    // string File = readChunkFromFile(indexPath);

    // const char *testResponse =
    // "HTTP/1.1 200 OK\r\n"
    // "Content-Type: text/plain\r\n"
    // "Content-Length: " + strlen(File.c_str()) + "\r\n";
    client.request.resp.fullResponse = client.request.resp.statusLine + "Content-Type: " + client.request.mimeType + "\r\n" + "Content-Length: " + contentLength + "\r\n\r\n" + fileContent;
    cout << "response is [ " << client.request.resp.fullResponse << " ]" << endl;
    send(client.cfd, client.request.resp.fullResponse.c_str(), client.request.resp.fullResponse.size(), 0);
    if (client.clientState == WRITING_DONE)
        client.clientState = SENDING_DONE;
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

void WebServ::dirList(string root, string location, Client &client)
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
        sessionKey = client.request.extractSessionId();
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
            string fileWithoutId = getOriginalFileName(client.request, subDirs[i], fileId);
            if (fileId != userId)
                continue;
            dirlist += "<li><a href=\"" + location + "/" + fileWithoutId + "\">" + fileWithoutId + "</a></li>";
        }
        else
            dirlist += "<li><a href=\"" + location + "/" + subDirs[i] + "\">" + subDirs[i] + "</a></li>";
    }

    dirlist += "</ul></body></html>";
    client.request.resp.setStatusLine("HTTP/1.1 200 OK\r\n");
    makeResponse(client, dirlist);
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
    // fileContent += readChunkFromFile("/home/akoraich/webserv/www/upload/files3.html");
    return fileContent;
}

bool WebServ::checkIndex(LocationNode node, Client &client, string location)
{
    string fileContent;

    for (unsigned long i = 0; i < node.index.size() ; i++)
    {
        if(node.root != "")
        {
            client.request.fullResource = node.root + "/" + node.index[i];
        }
        // if (location == "/upload")
        //     fileContent = uploadFile(node.root + "/" + node.index[i], node.root, client.request); // fiiiiix
        
        cout << "is dir and there is index  " << client.request.fullResource << endl;
        
        fileContent = readChunkFromFile(client);
        if (fileContent != "")
        {
            client.request.resp.setStatusLine("HTTP/1.1 200 OK\r\n");
            makeResponse(client, fileContent);
            return 0;
        }
    }
    throw HttpException(500, client);
}

// string getFilePath(Request req)
// {
//     string path;
//     while(req.resource.)
// }

// void WebServ::handleGetFile(Request req)
// {
//     req.resp.setStatusLine("HTTP/1.1 200 OK\r\n");
//     string fileContent = readChunkFromFile(req.fullResource);
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

void WebServ::handleGetFile(Client &client, map<string, string> &data)
{
    string fileContent;
    client.request.resp.setStatusLine("HTTP/1.0 200 OK\r\n");
    if (client.request.mimeType == "text/html")
    {
        cout << "is text" << endl;
        // if (data.empty())
            fileContent = readChunkFromFile(client);
        // else
        //     fileContent = dynamicRender(client.request.fullResource, data);
        makeResponse(client, fileContent);
    }
    else if (client.request.mimeType == "cgi")
    {
        client.request.mimeType = "text";
        fileContent = runCgi(client.request);
        makeResponse(client, fileContent);
    }
    else if (client.request.mimeType == "not supported")
    {
        client.request.mimeType = "text";
        fileContent = "this file type is not supported";
        makeResponse(client, fileContent);
    }
    else
    {
        cout << "Serving binary file: " << client.request.fullResource << endl;
    
        int filefd = open(client.request.fullResource.c_str(), O_RDONLY);
        if (filefd == -1) {
            cout << "Failed to open file: " << client.request.fullResource << endl;
            sendErrPageToClient(client.request.cfd, 404, client.request.serv);
            return;
        }
    
        // Get file size
        struct stat fileStat;
        if (fstat(filefd, &fileStat) == -1) {
            close(filefd);
            sendErrPageToClient(client.request.cfd, 500, client.request.serv);
            return;
        }

        // Send headers
        stringstream contentLength;
        contentLength << fileStat.st_size;
        
        string responseHeader = "HTTP/1.1 200 OK\r\n";
        responseHeader += "Content-Type: " + client.request.mimeType + "\r\n";
        responseHeader += "Content-Length: " + contentLength.str() + "\r\n\r\n";
        
        ssize_t headerSent = send(client.request.cfd, responseHeader.c_str(), responseHeader.size(), 0);
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
                ssize_t sent = send(client.request.cfd, buff + bytesSent, totalToSend - bytesSent, 0);
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
    return root + "/" + path;
}

void WebServ::requestChecks(Client &client, string &location, LocationNode &node)
{
    string target = client.request.getResource();
    cout << "target is " << target << endl;
    location = getLocation(client.request, client.request.serv);
    if (location == "")
        throw HttpException(404, client);
    node = client.request.serv.locationDict.find(location)->second;
    if (!exists(node.methods, client.request.getReqType()))
        throw HttpException(405, client);
    client.request.fullResource = getFullResource(node.root, location, target);
}

// void WebServ::handleGetUpload(Request req, LocationNode node, User loggedUser, string location)
// {
//     unsigned int userId = loggedUser.getId();
//     map <string, string> data = loggedUser.getKeyValData();
//     int i = req.fullResource.size();
//     while (i > 0)
//     {
//         if (req.fullResource[i - 1] == '/')
//            break;
//         i--;
//     }
//     string fileName = req.fullResource.substr(i);
//     string uploadResource = node.root + "/" + getFileNameWithUserId(req, userId ,fileName); //will need updates
//     if (isDirectory(req.fullResource) == true)
//     {
//         if (node.index.empty() == true || checkIndex(node, req, location) == 1)
//         {
//             if(node.autoIndex == true)
//                 dirList(node.root, location, req);
//             else
//                 throw ConfigException("forbidden request", 404);
//         }
//     }
//     else if (isRegularFile(uploadResource) == true)
//     {
//         req.fullResource = uploadResource;
//         cout << "should be fileeee" << endl;
//         getMimeType(req);
//         handleGetFile(req, data);
//     }
//     else
//         throw ConfigException("anaaa", 404);
// }

void WebServ::getMethode(Client &client)
{
    string sessionKey;
    map <string, string> data ;
    string location;
    LocationNode node;
    User loggedUser;
    unsigned int userId;
    try
    {
        requestChecks(client, location, node);
        //if location  is upload : 1. same as is protected 2. use encoding function (check if the user id is the same as the one in the file)
        client.request.fullResource = checkResource(client.request.fullResource, location);
        cout << " full resource is " << client.request.fullResource << endl;
        if (node.isProtected)
        {
            sessionKey = client.request.extractSessionId();
            if (!auth->isLoggedIn(sessionKey))
                throw HttpException(401, client);
            Session session = auth->sessions.find(sessionKey)->second;
            loggedUser = session.getUser();
            data = loggedUser.getKeyValData();
        }
        // if (location == "/upload")
        //     handleGetUpload(client.request, node, loggedUser, location);
        if (isDirectory(client.request.fullResource) == true)
        {
            if (node.index.empty() == true || checkIndex(node, client, location) == 1)
            {
                if(node.autoIndex == true)
                    dirList(node.root, location, client);
                else
                throw HttpException(404, client);
            }
        }
        if (isRegularFile(client.request.fullResource) == true)
        {
            cout << "wiwiwiwi file " << endl;
            getMimeType(client.request);
            handleGetFile(client, data);
        }
        else
        {
            cout << "nonononononon " << endl;
            throw HttpException(404, client);
        }
    }
    catch(HttpException& e)
    {
        Client client  = e.getClient();
        client.responseBuff = "error";
        client.clientState = WRITING_ERROR;
    }
}