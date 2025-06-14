#include "../../includes/webserv.hpp"

User::User()
{
    email = "";
    password = "";
}

User::User(string email, string password)
{
    this->email = email;
    this->password = password;
}