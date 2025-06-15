#include "../../includes/webserv.hpp"
#include "../../includes/Auth.hpp"
#include "../../includes/Debugger.hpp"

Session::Session(User &user) : user(user)
{
    this->user = user;
    this->key = generateSessionKey(user.getEmail());
    time_t now = time(0);
    if (now == -1)
    {
        cerr << "time error" << endl;
    }
    this->createdAt = now;
    this->expiredAt = now + (60 * 60); // after one hour
};

string& Session::getKey()
{
    return key;
}

User& Session::getUser()
{
    return user;
}



Session &Session::operator=(const Session &session)
{
    this->createdAt = session.createdAt;
    this->expiredAt = session.expiredAt;
    this->key = session.key;
    this->user = session.user;
    return (*this);
}

long int const &Session::getExpiredAt() const
{
    return expiredAt;
}
long int const &Session::getCreatedAt() const
{
    return createdAt;
}

// Simple MD5-like hash (not actual MD5, but similar concept)
string Session::generateSessionKey(const string &email)
{
    srand(time(NULL));
    string key = "";
    
    // Combine email hash + timestamp + random
    unsigned long hash = rand();
    for (size_t i = 0; i < email.length(); i++) {
        hash = hash * 31 + email[i];
    }
    
    // Add timestamp and random number
    time_t now = time(0);
    int randomNum = rand();
    
    // Convert to string
    ostringstream oss;
    oss << hash << "_" << now << "_" << randomNum;
    return oss.str();
}