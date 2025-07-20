#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"
#include "../../../includes/Exceptions.hpp"
#include <sstream>

void deleteFile(string path)
{
    if(remove(path.c_str()) == 0)
    {
        cout << "file deleted successfully" << endl;
        return;
    }
    cout << "didn't delete shit" << endl;
}

void WebServ::deleteMethod(Request &req, ServerNode &serv)
{
    string sessionKey;
    map <string, string> data ;
    string location;
    LocationNode node;
    try
    {
        requestChecks(req, serv, location, node);
        req.fullResource = checkResource(req.fullResource);
        cout << "resPath is [ " << req.fullResource << " ]" << endl;
        // if (isDirectory(req.fullResource) == true)
        // {
        // }
        if (isRegularFile(req.fullResource) == true)
        {
            // sessionKey = req.extractSessionId();
            // if (!auth->isLoggedIn(sessionKey))
            // {
            //     sendErrPageToClient(req.cfd, 401, serv);
            //     return ;
            // }
            // Session session = auth->sessions.find(sessionKey)->second;
            // User loggedUser = session.getUser();
            // data = loggedUser.getKeyValData();
            cout << "got here" << endl;
            deleteFile(req.fullResource);
            //check if user has authorischdchsd to delete this file
            //check if parent dir (root ig) is w_ok and x_ok
            //check same permissions in the actual file
        }
        else
        {
            throw ConfigException("forbidden request", 404);
        }
    }
    catch(ConfigException& e)
    {
        sendErrPageToClient(req.cfd, e.getErrorCode(), req.serv);
        std::cerr << e.what() << '\n';
    }
}