#include "../includes/webserv.hpp"

set <string> LocationNode::possibleMethods;

void WebServ::parseLocation(ServerNode &serverNode, ifstream &configFile, size_t &lineNum)
{
    string line;
    LocationNode locationNode;

    size_t posOfDelimiter;
    vector <string> tokens;

    getline(configFile, line);
    lineNum++;
    line = trimSpaces(line);
    
    if (line.size() > 0 && line != "{")
    {
        cout << "{" << line << "}" << endl;
        
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
        if (tokens.size() == 0)
        {

        }
        else if (tokens[0] == "methods")
        {
            for (size_t i = 1; i < tokens.size(); i++)
            {
                if (locationNode.possibleMethods.find(tokens[i]) == locationNode.possibleMethods.end())
                {
                    cerr << "syntax error, unknown method '" << tokens[i] << "' at line: " << lineNum << endl;
                    criticalErr = true;
                    return ;
                }
                locationNode.methods.insert(tokens[i]);
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
            }
        }
        getline(configFile, line);
        lineNum++;
        line = trimSpaces(line);
    }


    
    serverNode.locationNodes.push_back(locationNode);
    return ;
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
        cout << "{" << line << "}" << endl;
        
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
        if (tokens.size() == 0)
        {

        }
        else if (startsWith(line, "location"))
            parseLocation(servNode, configFile, lineNum);
        else if (tokens[0] == "listen")
        {
            if (tokens.size() != 2)
            {
                cerr << "listen syntax is wrong, 'listen [PORT]' at line: " << lineNum << endl;
                criticalErr = true;
                return servNode;
            }
            if (!strAllDigit(tokens[1]) || tokens[1].size() > 5 || tokens[1].size() < 1)
            {
                cerr << "listen syntax is wrong port must be digits only (1-5 digits) at line:" << lineNum << endl;
                criticalErr = true;
                return servNode;
            }
            istringstream port (tokens[1]);
            if (port.fail())
            {
                cerr << "listen port syntax is wrong port must be digits only (1- 6 digits) at line:" << lineNum << endl;
                criticalErr = true;
                return servNode;
            }
            port >> servNode.port;
        }
        else if (tokens[0] == "host")
        {
            if (tokens.size() != 2)
            {
                cerr << "host syntax is wrong, 'host [xxx.xxx.xxx.xxx]' at line: " << lineNum << endl;
                criticalErr = true;
                return servNode;
            }
            vector <string> ipVec;
            ipVec = split(tokens[1], '.');
            for (size_t i = 0; i < ipVec.size(); i++)
            {
                if (!strAllDigit(ipVec[i]))
                {
                    cerr << "host syntax is wrong, 'host ip must be all digits' at line: " << lineNum << endl;
                    criticalErr = true;
                    return servNode;
                }
            }
            if (ipVec.size() != 4 )
            {
                cerr << "host syntax is wrong, 'host [xxx.xxx.xxx.xxx]' at line: " << lineNum << endl;
                criticalErr = true;
                return servNode;
            }
            servNode.host = tokens[1];
        }
        else if (tokens[0] == "server_names")
        {
            if (tokens.size() == 1)
            {
                cerr << "server_names syntax is wrong, please enter at least one server name at line: " << lineNum << endl;
                criticalErr = true;
                return servNode;
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
                return servNode;
            }
            servNode.root = tokens[1];
        }
        else if (tokens[0] == "client_max_body_size")
        {
            if (tokens.size() != 2 || tokens[1].size() < 1)
            {
                cerr << "client_max_body_size syntax is wrong please provide a size in megabytes at line': " << lineNum << endl;
                criticalErr = true;
                return servNode;
            }
            char last = tokens[1][tokens[1].size() - 1];
            if (last != 'm' && last != 'M')
            {
                cerr << "client_max_body_size syntax is wrong, 'client_max_body_size [size](M-m) at line': " << lineNum << endl;
                criticalErr = true;
                return servNode;
            }
            string clientMaxSizeStr = tokens[1].substr(0, tokens[1].size() - 1);
            if (!strAllDigit(clientMaxSizeStr))
            {
                cerr << "client_max_body_size syntax is wrong, 'client_max_body_size [size](M-m)' at line: " << lineNum << endl;
                criticalErr = true;
                return servNode;
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
                return servNode;
            }
            ErrorPageNode errorPage;
            size_t i = 1;
            for (; i < tokens.size() - 1; i++)
            {
                if (!strAllDigit(tokens[i]) || tokens[i].size() != 3)
                {
                    cerr << "error_page syntax is wrong, error code must be digits only (3 digits) at line: " << lineNum << endl;
                    criticalErr = true;
                    return servNode;
                }
                istringstream errorCode (tokens[i]);
                if (errorCode.fail())
                {
                    cerr << "error_page syntax is wrong, error code must be digits only (3 digits) at line: " << lineNum << endl;
                    criticalErr = true;
                    return servNode;
                }

                short errorCodeShort;
                errorCode >> errorCodeShort;
                errorPage.codes.insert(errorCodeShort);
                errorPage.page = tokens[i];
            }
            servNode.errorNodes.push_back(errorPage);
        }
        else
        {
            cerr << "syntax error, unkown entry in server context: '" << tokens[0] << "' in the line:" << lineNum << endl;
            criticalErr = true;
            return servNode;
        }
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

void WebServ::parsing(char *filename)
{
    ifstream configFile;
    vector <ServerNode> serverNodes;
    configFile.open(filename);
    if (configFile.fail())
    {
        criticalErr = true;
        cerr << "Error happened opening the file" << endl;
        return ;
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
            return ;
        getline(configFile, line);
        lineNum++;
    }
}