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
        sendErrPageToClient(cfd, 404, req.serv);
        return ;
    }
    string dbPassword = users[email].getPassword();
    if (dbPassword != password)
    {
        cout << "password does not match" << endl;
        sendErrPageToClient(cfd, 403, req.serv);
        return ;
    }
    cout << "user logged in successfully" << endl;

    string response;
    Session newSession = Session(users[email]);
    string sessionKey = newSession.getKey();

    map <string, string> data = users[email].getKeyValData();
    string dashboardContent = dynamicRender("./www/auth/dashboard.html", data);
    sessions.insert(make_pair(sessionKey, newSession));

    response += "HTTP/1.1 " + ushortToStr(301) + " " + getStatusMessage(301) + " \r\n";
    response +=  "Content-Type: text/html\r\n";
    response +=  "Location: dashboard.html\r\n";
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
        sendErrPageToClient(cfd, 409, req.serv); // 409 conflict
        return ;
    }
    User newUser(fName, lName, userName, email, password);
    users[email] = newUser;
    string response;

    Session newSession = Session(newUser);
    string sessionKey = newSession.getKey();

    map <string, string> data = newUser.getKeyValData();

    string dashboardContent = dynamicRender("./www/auth/dashboard.html", data);
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
    string loginFile = readFromFile("./www/login/login.html");
    string response = "";

    response += "HTTP/1.1 " + ushortToStr(errorCode) + " " + getStatusMessage(errorCode) + " \r\n";
    response +=  "Content-Type: text/html\r\n";
    response +=  "Content-Length: " + ushortToStr(loginFile.size()) + "\r\n\r\n";
    response += loginFile;
    send(cfd, response.c_str(), response.length(), 0);
}

bool Auth::isLoggedIn(string sessionKey)
{
    return exists(sessions, sessionKey);
}

void Auth::redirectToPage(int cfd, string page, int errorCode)
{
    string response;
    string file = readFromFile(page);
    response += "HTTP/1.1 " + ushortToStr(errorCode) + " " + getStatusMessage(errorCode) + " \r\n";
    response +=  "Content-Type: text/html\r\n";
    response +=  "Content-Length: " + ushortToStr(file.size()) + "\r\n\r\n";
    response += file;
    send(cfd, response.c_str(), response.length(), 0);
}

// void Auth::redirectToError(int cfd, ServerNode &serv, map <string, string> &errorData)
// {
//     string defaultErrorPage = serv.errorNodes[]
//     string errorResponse = dynamicRender()
// }

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