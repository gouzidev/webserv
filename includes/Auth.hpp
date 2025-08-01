#ifndef AUTH_HPP
#define AUTH_HPP

using namespace std;

#include "../includes/webserv.hpp"
#include <map>
#include <string>

class User;
class Session;
class Request;
class ServerNode;
class WebServ;


class Auth
{
    public:
        char *generalErrorResponse;
        void login(int cfd, string email, string password, Request &req);
        void signup(int cfd, string fName, string lName, string userName, string email, string password, Request &req);
        void logout(int cfd, string sessionKey, ServerNode &serv);
        void cleanUpSessions();
        void redirectToPage(int cfd, string page, int errorCode);
        void redirectToLogin(int cfd, int errorCode);
        bool isLoggedIn(string sessionKey);
        Auth();
        map < string, User> &getUsers();
    public: 

        // email -> User
        map < string, User> users;

        // sessionKey -> Session
        map <string, Session> sessions;

};

#endif