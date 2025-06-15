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
    cout << "User with data : " << email << " created with id: " << id << endl;
    cout << "User count is now: " << userCount << endl;
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
    cout << "User with data : " << fName << " " << lName << " " << email << " " << password << " created with id: " << id << endl;
    cout << "User count is now: " << userCount << endl;
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
    cout << "User with data : " << fName << " " << lName << " " << email << " " << password << " created with id: " << id << endl;
    cout << "User count is now: " << userCount << endl;
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