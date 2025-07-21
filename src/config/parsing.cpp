#include "../../includes/webserv.hpp"
#include "../../includes/Debugger.hpp"
#include "../../includes/Exceptions.hpp"

bool validHost(string hostStr, size_t &lineNum)
{
    if (hostStr == "localhost")
        return true;
    vector <string> ipVec;
    ipVec = split(hostStr, '.');
    for (size_t i = 0; i < ipVec.size(); i++)
    {
        if (!strAllDigit(ipVec[i]))
            throw ConfigException("host syntax is wrong, 'host ip must be all digits' or 'localhost' at line: " + toString(lineNum), 400);
    }
    if (ipVec.size() != 4 )
        throw ConfigException("host syntax is wrong, 'host [xxx.xxx.xxx.xxx]' or 'localhost' at line: " + toString(lineNum), 400);
    return true;
}

bool validPath(string path)
{
    if (path.size() == 0)
        return false;
    return path[0] == '/';
}

void WebServ::handleLocationLine(LocationNode &locationNode, vector <string> &tokens, size_t &lineNum)
{
    if (tokens.size() == 0)
    {

    }
    else if (tokens[0] == "methods" || tokens[0] == "allowed_methods")
    {
        for (size_t i = 1; i < tokens.size(); i++)
        {
            
            string method = tokens[i];
            transform(method.begin(), method.end(), method.begin(), ::toupper); // rfc 5.1.1  The method is case-sensitive.
            if (locationNode.possibleMethods.find(method) == locationNode.possibleMethods.end())
                throw ConfigException("syntax error, unknown method '" + method + "' at line: " + toString(lineNum), 400);
            if (locationNode.methods.find(method) != locationNode.methods.end())
                throw ConfigException("syntax error, duplicate method '" + method + "' at line: " + toString(lineNum), 400);
            locationNode.methods.insert(method);
        }
    }
    else if (tokens[0] == "index")
    {
        if (tokens.size() < 2)
            throw ConfigException("syntax error for index, please provide a default index: 'index [default page]' at line: " + toString(lineNum), 400);
        for (size_t i = 1; i < tokens.size(); i++)
        {
            locationNode.index.push_back(tokens[i]);
        }
    }
    else if (tokens[0] == "root")
    {
        if (tokens.size() != 2)
            throw ConfigException("syntax error for root, please provide a root path: 'root [path]' at line: " + toString(lineNum), 400);
        if (locationNode.root != "")
            throw ConfigException("config error, can't have more than 1 root at line: " + toString(lineNum), 400);
        if (!validPath(tokens[1]) || !checkDir(tokens[1], R_OK))
            throw ConfigException("syntax error for root, please provide an absolute root valid path starting with '/' -> : 'root [path]' at line: " + toString(lineNum), 400);
        locationNode.root = tokens[1];
    }
    else if (tokens[0] == "autoindex")
    {
        if (tokens.size() == 2)
        {
            if (tokens[1] != "on" && tokens[1] != "off")
                throw ConfigException("syntax error for autoindex, 'autoindex [on/off]' at line: " + toString(lineNum), 400);
            locationNode.autoIndex = tokens[1] == "on" ? true : false;
        }
    }
    else if (tokens[0] == "redirect")
    {
        if (tokens.size() < 2)
            throw ConfigException("redirect syntax is wrong, : 'redirect [error code?] [page], at line: " + toString(lineNum), 400);
        if (tokens.size() == 3)
        {
            if (!strAllDigit(tokens[1]))
                throw ConfigException("redirect syntax is wrong, error code shall be an int, at line: " + toString(lineNum), 400);
            istringstream redirect (tokens[1]);
            if (redirect.fail())
                throw ConfigException("redirect syntax is wrong, error code must be digits only (3 digits) at line: " + toString(lineNum), 400);

            short redirectShort;
            redirect >> redirectShort;
            locationNode.redirect.first = redirectShort;
            locationNode.redirect.second = tokens[2];
        }
        else
        {
            locationNode.redirect.first = -1;
            locationNode.redirect.second = tokens[1];
        }
        
    }
    else if (tokens[0] == "upload_dir")
    {
        if (tokens.size() != 2)
            throw ConfigException("syntax error for upload_dir, 'upload_dir [path]' at line: " + toString(lineNum), 400);
        if (locationNode.uploadDir != "")
            throw ConfigException("config error, can't have more than 1 upload_dir at line: " + toString(lineNum), 400);
        if (!validPath(tokens[1]) || !checkDir(tokens[1], W_OK))
            throw ConfigException("config error for upload_dir, please provide an absolute upload_dir valid path starting with '/' -> : 'root [path]' at line: " + toString(lineNum), 400);
        locationNode.uploadDir = tokens[1];
    }
    else if (tokens[0] == "isProtected")
    {
        if (tokens.size() != 1)
            throw ConfigException("syntax error for isProtected, 'isProtected' at line: " + toString(lineNum), 400);
        locationNode.isProtected = true;
    }
    else if (tokens[0] == "needContentLen")
    {
        if (tokens.size() != 1)
            throw ConfigException("syntax error for needContentLen, 'needContentLen' at line: " + toString(lineNum), 400);
        locationNode.needContentLen = true;
    }

    // needContentLen
    // else if (tokens[0] == "client_max_body_size")
    // {
    //     if (tokens.size() != 2 || tokens[1].size() < 1)
    //         throw ConfigException("client_max_body_size syntax is wrong please provide a size in megabytes at line: " + toString(lineNum), 400);
    //     if (locationNode.clientMaxBodySize != 0)
    //         throw ConfigException("config error, can't have more than 1 client_max_body_size at line: " + toString(lineNum), 400);
    //     char last = tokens[1][tokens[1].size() - 1];
    //     if (last != 'm' && last != 'M')
    //         throw ConfigException("client_max_body_size syntax is wrong, 'client_max_body_size [size](M-m) at line': " + toString(lineNum), 400);
    //     string clientMaxSizeStr = tokens[1].substr(0, tokens[1].size() - 1);
    //     if (!strAllDigit(clientMaxSizeStr))
    //         throw ConfigException("client_max_body_size syntax is wrong, 'client_max_body_size [size](M-m)' at line: " + toString(lineNum), 400);
    //     istringstream clientMaxSize (clientMaxSizeStr);
    //     clientMaxSize >> locationNode.clientMaxBodySize;
    // }
    else if (tokens[0] == "cgi_path")
    {
        if (tokens.size() != 3)
            throw ConfigException("syntax error for cgi_path, 'cgi_path [ext] [path_to_exec]' at line: " + toString(lineNum), 400);
        if (locationNode.possibleCgiExts.find(tokens[1]) == locationNode.possibleCgiExts.end())
            throw ConfigException("syntax error, unkown extention '" + tokens[2] + "' at line: " + toString(lineNum), 400);
        if (locationNode.cgiExts.find(tokens[2]) != locationNode.cgiExts.end())
            throw ConfigException("syntax error, duplicate extention '" + tokens[2] + "' at line: " + toString(lineNum), 400);
        if (!validPath(tokens[2]) || !checkDir(tokens[2], X_OK))
            throw ConfigException("config error for cgi_path, please provide an absolute cgi_path valid path starting with '/' -> : 'root [path]' at line: " + toString(lineNum), 400);
        locationNode.cgiExts[tokens[1]] = tokens[2];
    }
    else
        throw ConfigException("syntax error, unkown entry in location context: '" + tokens[0] + "' in the line:" + toString(lineNum), 400);
}

bool WebServ::validateLocationStr(string &location, ServerNode &serverNode, size_t &lineNum)
{
    // already exists
    if (serverNode.locationDict.find(location) != serverNode.locationDict.end())
        throw ConfigException("config error for location, location at line: " + toString(lineNum) + " already exists", 500);
    else if (location.size() == 0 || location[0] != '/')
        throw ConfigException("syntax error for location, location must start with '/' at line: " + toString(lineNum), 500);
    return true;
}

void WebServ::parseLocation(ServerNode &serverNode, ifstream &configFile, string &line, size_t &lineNum)
{
    LocationNode locationNode;

    size_t posOfDelimiter;
    vector <string> tokens;

    tokens = split(line, ' ');
    if (tokens.size() != 2)
        throw ConfigException("syntax error for location, 'location [path]' at line: " + toString(lineNum), 500);

    string location = tokens[1];
    locationNode.path = location;
    validateLocationStr(location, serverNode, lineNum);
    getline(configFile, line);
    lineNum++;
    line = trimSpaces(line);


    if (line.size() > 0 && line != "{")
        throw ConfigException("syntax error, please enter \"{\" in the line: " + toString(lineNum), 500);

    getline(configFile, line);
    lineNum++;
    line = trimSpaces(line);
    
    while (!configFile.eof() && line != "}")
    {
        posOfDelimiter = line.find(';');
        if (posOfDelimiter == string::npos && line.size() > 0) // not found
            throw ConfigException("syntax error, please end with ';' in the line: " + toString(lineNum), 500);
        line = line.substr(0, posOfDelimiter);
        tokens = split(line, ' ');
        
        handleLocationLine(locationNode, tokens, lineNum);
        if (criticalErr)
            return ;

        getline(configFile, line);
        lineNum++;
        line = trimSpaces(line);
    }

    serverNode.locationNodes.push_back(locationNode);
    serverNode.locationDict[location] = locationNode;

    return ;
}

void WebServ::handleServerBlock(ServerNode &servNode, vector <string> &tokens, size_t &lineNum)
{
    if (tokens[0] == "listen")
    {
        if (tokens.size() != 2)
            throw ConfigException("listen syntax is wrong, 'listen [PORT]' at line: " + toString(lineNum), 400);
        if (servNode.port != 0)
            throw ConfigException("config error, can't have more than 1 listen block at line: " + toString(lineNum), 400);
        if (!strAllDigit(tokens[1]) || tokens[1].size() > 5 || tokens[1].size() < 1)
            throw ConfigException("listen syntax is wrong, port must be digits only (1-5 digits) at line: " + toString(lineNum), 400);
        istringstream port (tokens[1]);
        if (port.fail())
            throw ConfigException("listen syntax is wrong, port must be digits only (1-5 digits) at line: " + toString(lineNum), 400);
        port >> servNode.port;
        if (servNode.port < 1024 || servNode.port > 41951)
            throw ConfigException("listen syntax is wrong, port range is [1024 - 41951] at line: " + toString(lineNum), 400);
    }
    else if (tokens[0] == "host")
    {
        if (tokens.size() != 2)
            throw ConfigException("host syntax is wrong, 'host [xxx.xxx.xxx.xxx]' or 'localahost' at line: " + toString(lineNum), 400);
        if (servNode.hostIp != "")
            throw ConfigException("config error, can't have more than 1 host at line: " + toString(lineNum), 400);
        if (!validHost(tokens[1], lineNum))
            throw ConfigException("host syntax is wrong, 'host [xxx.xxx.xxx.xxx]' or 'localahost' at line: " + toString(lineNum), 400);
        if (tokens[1] == "localhost")
            servNode.hostIp = "127.0.0.1";
        else
            servNode.hostIp = tokens[1];
    }
    
    else if (tokens[0] == "root")
    {
        if (tokens.size() != 2)
            throw ConfigException("syntax error for root, please provide a root path: 'root [path]' at line: " + toString(lineNum), 400);
        if (servNode.root != "")
            throw ConfigException("config error, can't have more than 1 root at line: " + toString(lineNum), 400);
        if (!validPath(tokens[1]) || !checkDir(tokens[1], R_OK))
            throw ConfigException("config error for root, please provide an absolute root valid path starting with '/' -> : 'root [path]' at line: " + toString(lineNum), 400);
        servNode.root = tokens[1];
    }
    else if (tokens[0] == "errorFolder")
    {
        if (tokens.size() != 2)
            throw ConfigException("syntax error for error pages folder, please provide a error pages folder path: 'errorFolder [path]' at line: " + toString(lineNum), 400);
        if (servNode.errorFolder != "")
            throw ConfigException("config error, can't have more than 1 errorFolder at line: " + toString(lineNum), 400);
        if (!validPath(tokens[1]) || !checkDir(tokens[1], R_OK))
            throw ConfigException("config error for error pages folder, please provide an absolute error pages folder valid path starting with '/' -> : 'errorFolder [path]' at line: " + toString(lineNum), 400);
        servNode.errorFolder = tokens[1];
    }
    else if (tokens[0] == "error_page")
    {
        if (tokens.size() < 3)
            throw ConfigException("error_page syntax is wrong, please provide error codes and page: 'error_page [error codes ...] [page]' at line: " + toString(lineNum), 400);
        string errorPageStr = tokens[tokens.size() - 1];
        if (servNode.errorFolder == "")
            throw ConfigException("syntax error for error pages, please provide a error pages folder path: 'errorFolder [path]' at line: " + toString(lineNum) + " before using error pages.", 400);
        
        string errorPagePath = servNode.errorFolder + "/" + errorPageStr;
        if (!validPath(errorPagePath))
            throw ConfigException("config error for error_page, please provide an absolute error_page valid path starting with '/' -> : 'root [path]' at line: " + toString(lineNum), 400);
        
        size_t i = 1;
        for (; i < tokens.size() - 1; i++)
        {
            if (!strAllDigit(tokens[i]) || tokens[i].size() != 3)
                throw ConfigException("error_page syntax is wrong, error code must be digits only (3 digits) at line: " + toString(lineNum), 400);
            istringstream errorCode (tokens[i]);
            if (errorCode.fail())
                throw ConfigException("error_page syntax is wrong, error code must be digits only (3 digits) at line: " + toString(lineNum), 400);

            short errorCodeShort;
            errorCode >> errorCodeShort;
            if (errorCodeShort > 599 || errorCodeShort < 400)
                throw ConfigException("error code syntax is wrong, error code must be in range of [400-500] at line: " + toString(lineNum), 400);
            if (servNode.errorNodes.find(errorCodeShort) != servNode.errorNodes.end())
                throw ConfigException("config error, code at line: " + toString(lineNum) + " is already used" + toString(lineNum), 400);
            servNode.errorNodes[errorCodeShort] = errorPagePath;
        }
    }
    else if (tokens[0] == "authFolder")
    {
        if (tokens.size() != 2)
            throw ConfigException("syntax error for auth pages folder, please provide a auth pages folder path: 'authFolder [path]' at line: " + toString(lineNum), 400);
        if (servNode.authFolder != "")
            throw ConfigException("config error, can't have more than 1 authFolder at line: " + toString(lineNum), 400);
        if (!validPath(tokens[1]) || !checkDir(tokens[1], R_OK))
            throw ConfigException("config error for auth pages folder, please provide an absolute auth pages folder valid path starting with '/' -> : 'authFolder [path]' at line: " + toString(lineNum), 400);
        servNode.authFolder = tokens[1];
    }
    else if (tokens[0] == "client_max_body_size")
    {
        if (tokens.size() != 2 || tokens[1].size() < 1)
            throw ConfigException("client_max_body_size syntax is wrong please provide a size in megabytes at line: " + toString(lineNum), 400);
        if (servNode.clientMaxBodySize != 0)
            throw ConfigException("config error, can't have more than 1 client_max_body_size at line: " + toString(lineNum), 400);
        char last = tokens[1][tokens[1].size() - 1];
        if (last != 'm' && last != 'M')
            throw ConfigException("client_max_body_size syntax is wrong, 'client_max_body_size [size](M-m) at line': " + toString(lineNum), 400);
        string clientMaxSizeStr = tokens[1].substr(0, tokens[1].size() - 1);
        if (!strAllDigit(clientMaxSizeStr))
            throw ConfigException("client_max_body_size syntax is wrong, 'client_max_body_size [size](M-m)' at line: " + toString(lineNum), 400);
        istringstream clientMaxSize (clientMaxSizeStr);
        clientMaxSize >> servNode.clientMaxBodySize;
    }
    else
        throw ConfigException("syntax error, unkown entry in server context: '" + tokens[0] + "' in the line:" + toString(lineNum) + " is already used", 400);
}

bool WebServ::validateLocation(ServerNode &servNode, LocationNode &locationNode)
{
    if (locationNode.root == "")
        locationNode.root = servNode.root;
    if (locationNode.methods.size() == 0)
    {
        cout << locationNode.root << endl;
        locationNode.methods.insert("GET");
        locationNode.methods.insert("POST");
        locationNode.methods.insert("DELETE");
    }
    if (!checkDir(locationNode.root, R_OK))
        throw ConfigException("config error, root folder: " + locationNode.root + " isn't valid", 400);
    return true;
}

void WebServ::validateParsing()
{
    size_t i = 0;
    ServerNode servNode;
    // set <unsigned short> ports;
    ;
    while (i < servNodes.size())
    {
        ServerNode &servNode = servNodes[i];
        // if (exists(ports, servNode.port))
        // {
        //     cerr << "duplicate port " << servNode.port << ".. aborting." << endl;
        //     criticalErr = true;
        //     return; 
        // }
        if (servNode.port == 0)
            throw ConfigException("no listen block provided", 400);
        if (servNode.hostIp == "")
            throw ConfigException("no host block provided", 400);
        for (size_t i = 0; i < servNode.locationNodes.size(); i++)
        {
            LocationNode &localNode = servNode.locationNodes[i];
            validateLocation(servNode, localNode);
            servNode.locationDict[localNode.path] = localNode;  // ✅ Store updated version
        }
        if (!checkDir(servNode.root, R_OK))
            throw ConfigException("config error, root folder: " + servNode.root + " isn't valid", 400);
        string supposedDefaultErrorPage = servNode.errorFolder + "/" + "error.html";
        if (!checkFile(supposedDefaultErrorPage, O_RDONLY))
            throw ConfigException("config error, default error page: " + supposedDefaultErrorPage + " isn't valid or non existent", 400);
        servNode.defaultErrorPage = supposedDefaultErrorPage;
        hostServMap[servNode.hostIp + ":" + ushortToStr(servNode.port)] = servNode;
        i++;
    }
    
}

void  WebServ::handleServerLine(ServerNode &servNode, ifstream &configFile, vector <string> &tokens, string &line, size_t &lineNum)
{
    if (tokens.size() == 0 || criticalErr)
        return ;
    if (startsWith(line, "location"))
        parseLocation(servNode, configFile, line, lineNum);
    else
        handleServerBlock(servNode, tokens, lineNum);
}

ServerNode WebServ::parseServer(ifstream &configFile, size_t &lineNum)
{
    ServerNode servNode;
    string line;
    size_t posOfDelimiter;
    vector <string> tokens;

    getline(configFile, line);
    lineNum++;
    line = trimSpaces(line);
    
    if (line.size() > 0 && line != "{")
        throw ConfigException("syntax error, please enter \"{\" in the line: " + toString(lineNum), 500);
    getline(configFile, line);
    lineNum++;
    line = trimSpaces(line);
    while (!configFile.eof() && line != "}")
    {
        if (!startsWith(line, "location"))
        {
            posOfDelimiter = line.find(';');
            if (posOfDelimiter == string::npos && line.size() > 0) // not found
                throw ConfigException("syntax error, please end with ';' in the line: " + toString(lineNum), 500);
            line = line.substr(0, posOfDelimiter);
        }
        tokens = split(line, ' ');
        handleServerLine(servNode, configFile, tokens, line, lineNum);
        getline(configFile, line);
        lineNum++;
        line = trimSpaces(line);
    }
    if (line == "}")
    {
        getline(configFile, line);
        lineNum++;
        line = trimSpaces(line);
    }
    return servNode;
}

vector <ServerNode> WebServ::parsing(char *filename)
{
    ifstream configFile;
    vector <ServerNode> serverNodes;
    configFile.open(filename);
    if (configFile.fail())
        throw ConfigException("config file not found or can't be opened", 500);
    size_t lineNum = 0;
    string line;

    getline(configFile, line);
    lineNum++;
    while (!configFile.eof())
    {
        line = trimSpaces(line);
        if (line.size())
        {
            ServerNode server;
            server = parseServer(configFile, lineNum);
            serverNodes.push_back(server);
        }
        else if (line != "server")
            cerr << "syntax error, please enter \"server\" in the line" << endl;
        getline(configFile, line);
        lineNum++;
    }
    this->servNodes = serverNodes;
    validateParsing();

    return serverNodes;
}