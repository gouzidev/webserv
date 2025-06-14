#include "../includes/webserv.hpp"

class WebServ::Auth
{
    public:
        void login(Request &req, ServerNode &serv);
        
        Auth();

};