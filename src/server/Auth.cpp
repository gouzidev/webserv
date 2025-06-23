#include "../../includes/webserv.hpp"
#include "../../includes/Auth.hpp"
#include "../../includes/Exceptions.hpp"

Auth::Auth()
{
    generalErrorResponse = (char *)"HTTP/1.1 500 INTERNAL SERVER ERROR\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nServer Error";
    User admin("salah", "gouzi", "salahgouzi11@gmail.com", "1234");
    User asma("asma", "koraichi", "asma@gmail.com", "1234");
    users[admin.getEmail()] = admin;
    users[asma.getEmail()] = asma;
}

void Auth::login(int cfd, string email, string password, Request &req)
{
    if (!exists(users, email))
    {
        cout << "email not found in users" << endl;
        sendErrToClient(cfd, 404, req.serv);
        return ;
    }
    string dbPassword = users[email].getPassword();
    if (dbPassword != password)
    {
        cout << "password does not match" << endl;
        sendErrToClient(cfd, 403, req.serv);
        return ;
    }
    cout << "user logged in successfully" << endl;

    ifstream dashboardFile;
    dashboardFile.open("./www/auth/dashboard.html");
    if (dashboardFile.fail())
    {
        cerr << "Error happened opening the file of dashboard" << endl;
        send(cfd, generalErrorResponse, strlen(generalErrorResponse), 0);
        return ;
    }
    string line, dashboardContent = "";
    while (getline(dashboardFile, line))
    {
        dashboardContent += line + "\r\n";
    }
    string response;

    Session newSession = Session(users[email]);
    string sessionKey = newSession.getKey();
    sessions.insert(make_pair(sessionKey, newSession));  // ✅ This doesn't need default constructor

    response += "HTTP/1.1 " + ushortToStr(200) + " " + getStatusMessage(200) + " \r\n";
    response +=  "Content-Type: text/html\r\n";
    response += "Set-Cookie: sessionId=" + newSession.getKey() + "; Path=/; HttpOnly; Max-Age=3600\r\n";
    response +=  "Content-Length: " + ushortToStr(dashboardContent.size()) + "\r\n\r\n";
    response += dashboardContent;
    send(cfd, response.c_str(), response.length(), 0);
}


void Auth::signup(int cfd, string fName, string lName, string userName, string email, string password, Request &req)
{
    if (exists(users, email))
    {
        cout << "email already exists found in users" << endl;
        sendErrToClient(cfd, 409, req.serv); // 409 conflict
        return ;
    }

    User newUser(fName, lName, userName, email, password);
    users[email] = newUser;
    ifstream dashboardFile;
    dashboardFile.open("./www/auth/dashboard.html");
    if (dashboardFile.fail())
    {
        cerr << "Error happened opening the file of dashboard" << endl;
        send(cfd, generalErrorResponse, strlen(generalErrorResponse), 0);
        return ;
    }
    string line, dashboardContent = "";
    while (getline(dashboardFile, line))
    {
        dashboardContent += line + "\r\n";
    }
    string response;

    Session newSession = Session(newUser);
    string sessionKey = newSession.getKey();
    sessions.insert(make_pair(sessionKey, newSession));

    response += "HTTP/1.1 " + ushortToStr(200) + " " + getStatusMessage(200) + " \r\n";
    response +=  "Content-Type: text/html\r\n";
    response += "Set-Cookie: sessionId=" + newSession.getKey() + "; Path=/; HttpOnly; Max-Age=3600\r\n";
    response +=  "Content-Length: " + ushortToStr(dashboardContent.size()) + "\r\n\r\n";
    response += dashboardContent;
    send(cfd, response.c_str(), response.length(), 0);
}

void Auth::redirectToLogin(int cfd, int errorCode)
{
    ifstream loginFile;
    loginFile.open("./www/login/login.html");
    if (loginFile.fail())
    {
        cerr << "Error happened opening the file of login" << endl;
        send(cfd, generalErrorResponse, strlen(generalErrorResponse), 0);
        return ;
    }
    string line, loginFileContent = "";
    while (getline(loginFile, line))
    {
        loginFileContent += line + "\r\n";
    }
    string response;

    response += "HTTP/1.1 " + ushortToStr(errorCode) + " " + getStatusMessage(errorCode) + " \r\n";
    response +=  "Content-Type: text/html\r\n";
    response +=  "Content-Length: " + ushortToStr(loginFileContent.size()) + "\r\n\r\n";
    response += loginFileContent;
    send(cfd, response.c_str(), response.length(), 0);
}

void Auth::logout(int cfd, string sessionKey, ServerNode &serv)
{
    if (exists(sessions, sessionKey))
    {
        cout << "user logged out successfully" << endl;
        sessions.erase(sessionKey);
        redirectToLogin(cfd, 200);
    }
    else
    {
        cout << "session key not found" << endl;
        redirectToLogin(cfd, 401);
    }
}

map < string, User> &Auth::getUsers()
{
    return users;
}

void Auth::cleanUpSessions()
{
    map <string, Session>::iterator sessionsMapIt = sessions.begin();

    time_t now = time(0);
    while (sessionsMapIt != sessions.end())
    {
        Session session = sessionsMapIt->second;

        session = sessionsMapIt->second;
        if (now > session.getExpiredAt())
        {
            sessions.erase(session.getKey());
            cout << "Session expired and removed for user: " << session.getUser().getEmail() << endl;
        }
        sessionsMapIt++;
    }
}           