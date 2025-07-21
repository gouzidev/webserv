#include "../../includes/webserv.hpp"

User::User()
{
    id = userCount;
    email = "";
    password = "";
    userCount++;
}

User::User(string email, string password)
{
    id = userCount;
    this->firstName = "";
    this->lastName = "";
    this->email = email;
    this->password = password;
    userCount++;
}

User::User(string fName, string lName, string email, string password)
{
    id = userCount;
    this->userName = "";
    this->firstName = fName;
    this->lastName = lName;
    this->email = email;
    this->password = password;
    userCount++;
}

User::User(string fName, string lName, string userName, string email, string password)
{
    id = userCount;
    this->userName = userName;
    this->firstName = fName;
    this->lastName = lName;
    this->email = email;
    this->password = password;
    userCount++;
}

const unsigned int &User::getId() const
{
    return id;
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

map <string, string> User::getKeyValData()
{
    map <string, string> data;
    data["firstName"] = firstName;
    data["lastName"] = lastName;
    data["email"] = email;
    data["userName"] = userName;

    return data;
}
