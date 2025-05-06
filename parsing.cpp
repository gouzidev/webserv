#include "webserv.hpp"

ServerNode WebServ::parseServer()
{
    ServerNode serverNode;

}

void WebServ::parsing(char *filename)
{
    std::ifstream configFile;
    configFile.open(filename);
    if (configFile.fail())
    {
        criticalErr = true;
        cerr << "Error happened opening the file" << endl;
        return ;
    }
    cout << "opened file successully" << endl;


    string line;
    while (getline(configFile, line))
    {
        string trimmed = trimSpaces(line);
        if ((line.size() > 4) && trimmed.substr(0, 5) == "server")
        {
            
        }
    }
    

}