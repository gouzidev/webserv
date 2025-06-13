#include "../../includes/webserv.hpp"
#include "../../includes/Debugger.hpp"

bool validHost(string hostStr, size_t &lineNum)
{
    vector <string> ipVec;
    ipVec = split(hostStr, '.');
    // for (size_t i = 0; i < ipVec.size(); i++)
    // {
    //     if (!strAllDigit(ipVec[i]))
    //     {
    //         cerr << "host syntax is wrong, 'host ip must be all digits' at line: " << lineNum << endl;
    //         return false;
    //     }
    // }
    // if (ipVec.size() != 4 )
    // {
    //     cerr << "host syntax is wrong, 'host [xxx.xxx.xxx.xxx]' at line: " << lineNum << endl;
    //     return false;
    // } 

    // just for now, every host will be a valid host
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
            {
                cerr << "syntax error, unknown method '" << method << "' at line: " << lineNum << endl;
                criticalErr = true;
                return ;
            }
            if (locationNode.methods.find(method) != locationNode.methods.end())
            {
                cerr << "syntax error, duplicate method '" << method << "' at line: " << lineNum << endl;
                criticalErr = true;
                return ;
            }
            locationNode.methods.insert(method);
        }
    }
    else if (tokens[0] == "index")
    {
        if (tokens.size() < 2)
        {
            cerr << "syntax error for index, please provide a default index: 'index [default page]' at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        for (size_t i = 1; i < tokens.size(); i++)
        {
            locationNode.index.push_back(tokens[i]);
        }
    }
    else if (tokens[0] == "root")
    {
        if (tokens.size() != 2)
        {
            cerr << "syntax error for root, please provide a root path: 'root [path]' at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        if (locationNode.root != "")
        {
            cerr << "config error, can't have more than 1 root at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        if (!validPath(tokens[1]) || !checkDir(tokens[1], R_OK))
        {
            cerr << "syntax error for root, please provide an absolute root valid path starting with '/' -> : 'root [path]' at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        locationNode.root = tokens[1];
    }
    else if (tokens[0] == "autoindex")
    {
        if (tokens.size() == 2)
        {
            if (tokens[1] != "on" && tokens[1] != "off")
            {
                cerr << "syntax error for autoindex, 'autoindex [on/off]' at line: " << lineNum << endl;
                criticalErr = true;
                return ;
            }
            locationNode.autoIndex = tokens[1] == "on" ? true : false;
        }
    }
    else if (tokens[0] == "redirect")
    {
        if (tokens.size() < 2)
        {
            cerr << "redirect syntax is wrong, : 'redirect [error code?] [page], at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        if (tokens.size() == 3)
        {
            if (!strAllDigit(tokens[1]))
            {
                cerr << "redirect syntax is wrong, error code shall be an int, at line: " << lineNum << endl;
                criticalErr = true;
                return ;
            }
            istringstream redirect (tokens[1]);
            if (redirect.fail())
            {
                cerr << "redirect syntax is wrong, error code must be digits only (3 digits) at line: " << lineNum << endl;
                criticalErr = true;
                return ;
            }

            short redirectShort;
            redirect >> redirectShort;
            locationNode.redirect.first = redirectShort;
            locationNode.redirect.second = tokens[2];
        }
        else
        {
            locationNode.redirect.second = tokens[1];
            locationNode.redirect.first = -1;
        }
        
    }
    else if (tokens[0] == "upload_dir")
    {
        if (tokens.size() != 2)
        {
            cerr << "syntax error for upload_dir, 'upload_dir [path]' at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        if (locationNode.upload_path != "")
        {
            cerr << "config error, can't have more than 1 upload_dir at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        if (!validPath(tokens[1]) || !checkDir(tokens[1], W_OK))
        {
            cerr << "config error for upload_dir, please provide an absolute upload_dir valid path starting with '/' -> : 'root [path]' at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        locationNode.upload_path = tokens[1];
    }
    else if (tokens[0] == "cgi_path")
    {
        if (tokens.size() != 3)
        {
            cerr << "syntax error for cgi_path, 'cgi_path [ext] [path_to_exec]' at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        if (locationNode.possibleCgiExts.find(tokens[1]) == locationNode.possibleCgiExts.end())
        {
            cerr << "syntax error, unkown extention '" << tokens[1] << "' at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }

        if (locationNode.cgi_exts.find(tokens[2]) != locationNode.cgi_exts.end())
        {
            cerr << "syntax error, duplicate extention '" << tokens[2] << "' at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        if (!validPath(tokens[2]) || !checkDir(tokens[2], X_OK))
        {
            cerr << "config error for cgi_path, please provide an absolute cgi_path valid path starting with '/' -> : 'root [path]' at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        locationNode.cgi_exts[tokens[1]] = tokens[2];
    }
    else
    {
        cerr << "syntax error, unkown entry in location context: '" << tokens[0] << "' in the line:" << lineNum << endl;
        criticalErr = true;
        return ;
    }
}

bool WebServ::validateLocationStr(string &location, ServerNode &serverNode, size_t &lineNum)
{
    // already exists
    if (serverNode.locationDict.find(location) != serverNode.locationDict.end())
    {
        cerr << "config error for location, location at line: " << lineNum << " already exists" << endl;
        criticalErr = true;
        return false;
    }
    else if (location.size() == 0 || location[0] != '/')
    {
        cerr << "syntax error for location, location 'location [path] at line: " << lineNum << endl;
        criticalErr = true;
        return false;
    }
    return true;
}

void WebServ::parseLocation(ServerNode &serverNode, ifstream &configFile, string &line, size_t &lineNum)
{
    LocationNode locationNode;

    size_t posOfDelimiter;
    vector <string> tokens;

    tokens = split(line, ' ');
    if (tokens.size() != 2)
    {
        cerr << "syntax error for location, 'location [path]' at line: " << lineNum << endl;
        criticalErr = true;
        return ;
    }

    string location = tokens[1];
    if (!validateLocationStr(location, serverNode, lineNum))
        return ;
    getline(configFile, line);
    lineNum++;
    line = trimSpaces(line);


    if (line.size() > 0 && line != "{")
    {
        cerr << "syntax error, please enter \"{\" in the line:" << lineNum << endl;
        criticalErr = true;
        return ;
    }

    getline(configFile, line);
    lineNum++;
    line = trimSpaces(line);
    
    while (!configFile.eof() && line != "}")
    {
        posOfDelimiter = line.find(';');
        if (posOfDelimiter == string::npos && line.size() > 0) // not found
        {
            cerr << "syntax error, please end with ';' in the line:" << lineNum << endl;
            criticalErr = true;
            return ;
        }
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
        {
            cerr << "listen syntax is wrong, 'listen [PORT]' at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        if (servNode.port != 0)
        {
            cerr << "config error, can't have more than 1 listen block at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        if (!strAllDigit(tokens[1]) || tokens[1].size() > 5 || tokens[1].size() < 1)
        {
            cerr << "listen syntax is wrong port must be digits only (1-5 digits) at line:" << lineNum << endl;
            criticalErr = true;
            return ;
        }
        istringstream port (tokens[1]);
        if (port.fail())
        {
            cerr << "listen syntax is wrong, port must be digits only (1- 6 digits) at line:" << lineNum << endl;
            criticalErr = true;
            return ;
        }
        port >> servNode.port;
        if (servNode.port < 1024 || servNode.port > 41951)
        {
            cerr << "listen syntax is wrong, port range is [1024 - 41951] at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
    }
    else if (tokens[0] == "host")
    {
        if (tokens.size() != 2)
        {
            cerr << "host syntax is wrong, 'host [xxx.xxx.xxx.xxx]' at line: " << lineNum << endl;
            criticalErr = true;
        }
        if (servNode.hostStr != "")
        {
            cerr << "config error, can't have more than 1 host at line: " << lineNum << endl;
            criticalErr = true;
        }
        if (validHost(tokens[1], lineNum))
        {
            servNode.hostStr = tokens[1];
        }
        else
            criticalErr = true;
    }
    else if (tokens[0] == "server_names")
    {
        if (tokens.size() == 1)
        {
            cerr << "server_names syntax is wrong, please enter at least one server name at line: " << lineNum << endl;
            criticalErr = true;
        }
        set <string> server_names;
        for (size_t i = 1; i < tokens.size(); i++)
        {
            server_names.insert(tokens[i]);
        }
        servNode.serverNames = server_names;
    }
    else if (tokens[0] == "root")
    {
        if (tokens.size() != 2)
        {
            cerr << "root syntax is wrong, 'root [path]' at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        if (servNode.root != "")
        {
            cerr << "config error, can't have more than 1 root at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        if (!validPath(tokens[1]) || !checkDir(tokens[1], R_OK))
        {
            cerr << "syntax error for root, please provide an absolute root valid path starting with '/' -> : 'root [path]' at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        servNode.root = tokens[1];
    }
    else if (tokens[0] == "client_max_body_size")
    {
        if (tokens.size() != 2 || tokens[1].size() < 1)
        {
            cerr << "client_max_body_size syntax is wrong please provide a size in megabytes at line': " << lineNum << endl;
            criticalErr = true;
        }
        if (servNode.clientMaxBodySize != 0)
        {
            cerr << "config error, can't have more than 1 client_max_body_size at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        char last = tokens[1][tokens[1].size() - 1];
        if (last != 'm' && last != 'M')
        {
            cerr << "client_max_body_size syntax is wrong, 'client_max_body_size [size](M-m) at line': " << lineNum << endl;
            criticalErr = true;
        }
        string clientMaxSizeStr = tokens[1].substr(0, tokens[1].size() - 1);
        if (!strAllDigit(clientMaxSizeStr))
        {
            cerr << "client_max_body_size syntax is wrong, 'client_max_body_size [size](M-m)' at line: " << lineNum << endl;
            criticalErr = true;
        }
        istringstream clientMaxSize (clientMaxSizeStr);
        clientMaxSize >> servNode.clientMaxBodySize;
    }
    else if (tokens[0] == "error_page")
    {
        if (tokens.size() < 3)
        {
            cerr << "error_page syntax is wrong, : 'error_page [error codes ...] [page], at line: " << lineNum << endl;
            criticalErr = true;
        }
        string errorPageStr = tokens[tokens.size() - 1];
        if (!validPath(errorPageStr) || !checkDir(errorPageStr, R_OK))
        {
            cerr << "config error, for error_page '" << errorPageStr << "' please provide an absolute error_page valid path starting with '/' -> : 'root [path]' at line: " << lineNum << endl;
            criticalErr = true;
            return ;
        }
        
        size_t i = 1;
        for (; i < tokens.size() - 1; i++)
        {
            if (!strAllDigit(tokens[i]) || tokens[i].size() != 3)
            {
                cerr << "error_page syntax is wrong, error code must be digits only (3 digits) at line: " << lineNum << endl;
                criticalErr = true;
                return ;
            }
            istringstream errorCode (tokens[i]);
            if (errorCode.fail())
            {
                cerr << "error_page syntax is wrong, error code must be digits only (3 digits) at line: " << lineNum << endl;
                criticalErr = true;
                return ;
            }

            short errorCodeShort;
            errorCode >> errorCodeShort;
            if (errorCodeShort > 599 || errorCodeShort < 400)
            {
                cerr << "error code syntax is wrong, error code must be in range of [400-500] at line: " << lineNum << endl;
                criticalErr = true;
                return ;
            }
            if (servNode.errorNodes.find(errorCodeShort) != servNode.errorNodes.end())
            {
                cerr << "config error, code at line: " << lineNum << " is already used" <<  endl;
                criticalErr = true;
                return ;
            }
            servNode.errorNodes[errorCodeShort] = errorPageStr;
        }
    }
    else
    {
        cerr << "syntax error, unkown entry in server context: '" << tokens[0] << "' in the line:" << lineNum << endl;
        criticalErr = true;
    }
}

bool WebServ::validateLocation(ServerNode &servNode, LocationNode &locationNode)
{
    if (locationNode.root == "")
        locationNode.root = servNode.root;
    if (locationNode.methods.size() == 0)
    {
        cerr << "syntax error, please specify allowed_methods: [allowed_methods {GET-POST-DELETE}] " << endl;
        criticalErr = true;
        return false;
    }
    if (!checkDir(locationNode.root, R_OK))
    {
        cerr << "config error, root folder " << locationNode.root << " isn't valid" << endl;
        criticalErr = true;
        return false;
    }
    return true;
}

void WebServ::validateParsing()
{
    size_t i = 0;
    ServerNode servNode;
    // set <unsigned short> ports;
    LocationNode localNode;
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
        {
            cerr << "no listen block provided" << endl;
            criticalErr = true;
            return ;
        }
        if (servNode.hostStr == "")
        {
            cerr << "no host block provided" << endl;
            criticalErr = true;
            return ;
        }
        for (size_t i = 0; i < servNode.locationNodes.size(); i++)
        {
            localNode = servNode.locationNodes[i];
            if (!validateLocation(servNode, localNode))
                return ;
        }
        if (!checkDir(servNode.root, R_OK))
        {
            cerr << "config error, root folder " << servNode.root << " isn't valid" << endl;
            criticalErr = true;
            return ;
        }
        set <string> servNames = servNode.serverNames;
        for (set <string>::iterator it = servNames.begin() ; it != servNames.end(); it++)
        {
            string servName = *it;
            string servNamePort = servName + ":" + ushortToStr(servNode.port);
            if (servNameServMap.find(servNamePort) != servNameServMap.end())
            {
                ServerNode foundServ = servNameServMap.find(servNamePort)->second;
                if (foundServ.hostStr == servNode.hostStr)
                {
                    cerr << "duplicate server name for the same port: " << servNamePort << endl;
                    criticalErr = true;
                    return ;
                }
            }
            servNameServMap[servNamePort] = servNode;
        }
        hostServMap[servNode.hostStr + ":" + ushortToStr(servNode.port)] = servNode;
        // ports.insert(servNode.port);
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
    {
        cerr << "syntax error, please enter \"{\" in the line:" << lineNum << endl;
        criticalErr = true;
        return servNode;
    }
    getline(configFile, line);
    lineNum++;
    line = trimSpaces(line);
    while (!configFile.eof() && line != "}")
    {
        if (!startsWith(line, "location"))
        {
            posOfDelimiter = line.find(';');
            if (posOfDelimiter == string::npos && line.size() > 0) // not found
            {
                cerr << "syntax error, please end with ';' in the line:" << lineNum << endl;
                criticalErr = true;
                return servNode;
            }
            line = line.substr(0, posOfDelimiter);
        }
        tokens = split(line, ' ');
        handleServerLine(servNode, configFile, tokens, line, lineNum);
        if (criticalErr)
            return servNode;
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
    {
        criticalErr = true;
        cerr << "Error happened opening the file" << endl;
        return serverNodes;
    }
    size_t lineNum = 0;
    string line;

    getline(configFile, line);
    lineNum++;
    while (!configFile.eof() && !criticalErr)
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
        if (criticalErr)
            return serverNodes;
        getline(configFile, line);
        lineNum++;
    }
    this->servNodes = serverNodes;
    validateParsing();

    return serverNodes;
}