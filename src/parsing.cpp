#include "../includes/webserv.hpp"

// LocationNode WebServ::parseLocation(ServerNode &serverNode, ifstream &configFile, string &line, size_t &lineI)
// {

// }


ServerNode WebServ::parseServer(ifstream &configFile)
{
    ServerNode servNode;
    string line;
    getline(configFile, line);
    line = trimSpaces(line);
    if (line != "{")
        cout << "parsing error, please enter \"{\" in the first line" << endl;
    parseServer(configFile);
    getline(configFile, line);
    cout << line << endl;

    return servNode;
}

void WebServ::parsing(char *filename)
{
    ifstream configFile;
    configFile.open(filename);
    if (configFile.fail())
    {
        criticalErr = true;
        cerr << "Error happened opening the file" << endl;
        return ;
    }
    cout << "opened file successully" << endl;


    string line;

    getline(configFile, line);
    while (!configFile.eof())
    {
        line = trimSpaces(line);
        if (line != "server")
            cout << "parsing error, please enter \"server\" in the first line" << endl;
            
        else
            parseServer(configFile);
    }
}