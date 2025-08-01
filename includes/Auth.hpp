#ifndef AUTH_HPP
#define AUTH_HPP


#include "../includes/webserv.hpp"
// containers
#include <map>
#include <set>
#include <vector>

#include <string>

using namespace std;


class Request;
class ServerNode;
class WebServ;
class Client;


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
        void login(Client &client, string email, string password);
        void signup(Client &client, string fName, string lName, string userName, string email, string password);
        void logout(Client &client, string sessionKey);
        void cleanUpSessions();
        void redirectToPage(Client &client, string page, int errorCode);
        void redirectToLogin(Client &client, int errorCode);
        bool isLoggedIn(string sessionKey);
        string getRedirectionRequest(Client &client, unsigned short statusCode, string location);
        Auth();
        map < string, User> &getUsers();
    public: 

        // email -> User
        map < string, User> users;

        // sessionKey -> Session
        map <string, Session> sessions;

};

#endif