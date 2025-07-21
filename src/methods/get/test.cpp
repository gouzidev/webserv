#include "../../../includes/webserv.hpp"

string getFileNameWithUserId(unsigned int userId, string originalName)
{
    stringstream ss;
    
    ss << userId;
    string userIdStr = ss.str();
    int i = userIdStr.size();
    while (i++ < 3)
    {
        userIdStr.insert(0, "0");
    }
    string resName = userIdStr + "_" + originalName;
    return resName;
}

string getOriginalFileName(string fileNameWithUserId, unsigned int &userIdAssociated)
{
    userIdAssociated = -1;
    short namePrefixSize = 3 + 1; // size of ([3] + ['_'])

    string userIdFromFileName = fileNameWithUserId.substr(0, 3);

    userIdAssociated = stoi(userIdFromFileName);
    cout << "User ID associated with file: " << userIdAssociated << endl;
    string originalName = fileNameWithUserId.substr(namePrefixSize + 1);
    return originalName;
}
int main()
{
    unsigned int userId = 999;
    string originalName = "SAMIR HBIBNA.txt";

    string encoded = getFileNameWithUserId(userId, originalName);

    unsigned int userIdTest;
    string decoded = getOriginalFileName(encoded, userIdTest);

    cout << "Encoded: " << encoded << endl;
    cout << "Decoded: " << decoded << endl;
}