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

void Auth::login(Client &client, string email, string password)
{
    Request &req = client.request;
    int cfd = client.cfd;

    if (!exists(users, email))
    {

        cout << "email not found in users" << endl;
        throw HttpException(404, client);
    }
    string dbPassword = users[email].getPassword();
    if (dbPassword != password)
    {
        cout << "password does not match" << endl;
        throw HttpException(401, client);
        return ;
    }

    string response;
    Session newSession = Session(users[email]);
    string sessionKey = newSession.getKey();

    sessions.insert(make_pair(sessionKey, newSession));

    response += "HTTP/1.1 " + ushortToStr(301) + " " + getStatusMessage(301) + " \r\n";
    response +=  "Location: /auth/dashboard.html\r\n";
    response += "Set-Cookie: sessionId=" + newSession.getKey() + "; Path=/; HttpOnly; Max-Age=3600\r\n";
    response += "Content-Length: 0\r\n\r\n";
    client.responseBuff = response;
}

void Auth::signup(Client &client, string fName, string lName, string userName, string email, string password)
{
    Request &req = client.request;
    int cfd = client.cfd;

    if (exists(users, email))
    {
        cout << "email already exists found in users" << endl;
        throw HttpException(409, client);
    }
    User newUser(fName, lName, userName, email, password);
    users[email] = newUser;
    string response;

    Session newSession = Session(newUser);
    string sessionKey = newSession.getKey();

    sessions.insert(make_pair(sessionKey, newSession));
    response += "HTTP/1.1 " + ushortToStr(301) + " " + getStatusMessage(301) + " \r\n";
    response +=  "Content-Type: text/html\r\n";
    response +=  "Location: /auth/dashboard.html\r\n";
    response += "Set-Cookie: sessionId=" + newSession.getKey() + "; Path=/; HttpOnly; Max-Age=3600\r\n\r\n";
    client.responseBuff = response;
}

void Auth::redirectToLogin(Client &client, int errorCode)
{
    Request &req = client.request;
    int cfd = client.cfd;

    string loginFile = readFromFile("./www/login/login.html"); // must be changed !
    string response = "";

    response += "HTTP/1.1 " + ushortToStr(errorCode) + " " + getStatusMessage(errorCode) + " \r\n";
    response +=  "Content-Type: text/html\r\n";
    response +=  "Content-Length: " + ushortToStr(loginFile.size()) + "\r\n\r\n";
    response += loginFile;
    client.responseBuff = response;
}

bool Auth::isLoggedIn(string sessionKey)
{
    return exists(sessions, sessionKey);
}

void Auth::redirectToPage(Client &client, string page, int errorCode)
{
    Request &req = client.request;
    int cfd = client.cfd;

    string response;
    string file = readFromFile(page);
    response += "HTTP/1.1 " + ushortToStr(errorCode) + " " + getStatusMessage(errorCode) + " \r\n";
    response +=  "Content-Type: text/html\r\n";
    response +=  "Content-Length: " + ushortToStr(file.size()) + "\r\n\r\n";
    response += file;
    client.responseBuff = response;
}

// void Auth::redirectToError(Client &client, ServerNode &serv, map <string, string> &errorData)
// {
//     string defaultErrorPage = serv.errorNodes[]
//     string errorResponse = dynamicRender()
// }

void Auth::logout(Client &client, string sessionKey)
{
    Request &req = client.request;
    int cfd = client.cfd;

    if (exists(sessions, sessionKey))
    {
        cout << "user logged out successfully" << endl;
        sessions.erase(sessionKey);
        redirectToLogin(client, 200);
    }
    else
    {
        cout << "session key not found" << endl;
        redirectToLogin(client, 401);
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