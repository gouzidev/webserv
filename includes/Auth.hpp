#ifndef AUTH_HPP
#define AUTH_HPP

using namespace std;

#include "../includes/webserv.hpp"
// containers
#include <map>
#include <set>
#include <vector>

#include <string>

class Request;
class ServerNode;
class WebServ;


class User
{
    private:
        static unsigned int userCount;
        unsigned int id;
        string email;
        string userName;
        string firstName;
        string lastName;
        string password;
        vector <string> uploads;

    public:
        User();
        User(string email, string password);
        User(string fName, string lName, string email, string password);
        User(string fName, string lName, string userName, string email, string password);
        map<string, string> getKeyValData();
        const unsigned int &getId() const;
        const string &getEmail() const;
        const string &getFirstName() const;
        const string &getLastName() const;
        const string &getPassword() const;

        void setEmail(string str);
        void setFirstName(string str);
        void setLastName(string str);
        void setPassword(string str);
};

class Session
{
    private:
        User &user;
        string key;
        long int expiredAt; // in seconds
        long int createdAt; // in seconds
    public:
        Session(User &user);
        User &getUser();
        string &getKey();
        long int const &getExpiredAt() const;
        long int const &getCreatedAt() const;
        string generateSessionKey(const string &email);
        Session &operator=(const Session &session);
};

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