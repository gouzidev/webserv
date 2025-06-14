#include "../../includes/webserv.hpp"

User::User()
{
    email = "";
    password = "";
}

User::User(string email, string password)
{
    this->firstName = "";
    this->lastName = "";
    this->email = email;
    this->password = password;
}

User::User(string fName, string lName, string email, string password)
{
    this->firstName = fName;
    this->lastName = lName;
    this->email = email;
    this->password = password;
}

const string &User::getEmail() const
{
    return email;
}

const string &User::getFirstName() const
{
    return firstName;
}

const string &User::getLastName() const
{
    return lastName;
}

const string &User::getPassword() const
{
    return password;
}

void User::setEmail(string str)
{
    email = str;
}
void User::setFirstName(string str)
{
    firstName = str;
}
void User::setLastName(string str)
{
    lastName = str;
}
void User::setPassword(string str)
{
    password = str;
}