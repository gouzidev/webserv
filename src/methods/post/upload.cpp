#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"
#include "../../../includes/Exceptions.hpp"

void WebServ::handleUplaod(Request &req, long contentLen, ServerNode &servNode, LocationNode &locationNode)
{
    cout << "content len -> "  << contentLen << " bytes to upload" << endl;

    string errorRes;
    if (locationNode.uploadDir == "")
    {
        errorRes  = getErrorResponse(405, ""); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
        // error (no upload path provided)
    }
    string &contentType = req.headers["content-type"];
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == string::npos)
    {
        sendErrPageToClient(req.cfd, 400, servNode);
        return ;
    }
    string boundary = contentType.substr(boundaryPos + 9);
    string startBoundary = "--" + boundary;
    string expectedStart = startBoundary + "\r\n";
    string endBoundary = "\r\n" + startBoundary + "--";
    string body = req.body;

    cout << "expected start boundary -> '" << expectedStart << "'" << endl;
    cout << "expected substring      -> '" << body.substr(0, expectedStart.size()) << "'" << endl;
    if (body.substr(0, expectedStart.size()) != expectedStart)
    {
        // errorRes  = getErrorResponse(405, ""); // method not allowed 
        cerr << "error with startBoundary -> '" << startBoundary << "'" << endl;
        sendErrPageToClient(req.cfd, 400, servNode);
        return ;
    }
    
    size_t cdPos = body.find("Content-Disposition:", startBoundary.size() + 2); // content disposition (start) pos
    size_t cdnlPos = body.find("\r\n", cdPos);  // content disposition new line (end) pos (and start of body content type after ..)
    string contentDisposition = body.substr(cdPos, cdnlPos - cdPos);
    size_t i = 0;
    string name;
    string filename;
    while (i < contentDisposition.size())
    {
        if (contentDisposition.substr(i, 5) == "name=")
        {
            size_t firstQuote = -1;
            size_t lastQuote = -1;
            size_t j = i + 5;
            while (j < contentDisposition.size() && contentDisposition[j] != ';' && contentDisposition[j] != '\r' && contentDisposition[j] != '\n')
            {
                if (contentDisposition[j] == '"')
                {
                    if (firstQuote == -1)
                        firstQuote = j;
                    else
                        lastQuote = j;
                }
                j++;
            }
            name = contentDisposition.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            i = j;
            continue;
        }
        else if (contentDisposition.substr(i, 9) == "filename=")
        {
            size_t firstQuote = -1;
            size_t lastQuote = -1;
            size_t j = i + 9;
            while (j < contentDisposition.size() && contentDisposition[j] != ';' && contentDisposition[j] != '\r' && contentDisposition[j] != '\n')
            {
                if (contentDisposition[j] == '"')
                {
                    if (firstQuote == -1)
                        firstQuote = j;
                    else
                        lastQuote = j;
                }
                j++;
            }
            filename = contentDisposition.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            i = j;
            continue;
        }
        i++;
    }
    string path = locationNode.uploadDir;
    string newFile = path + "/" + filename;

    int fd = open(newFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1)
    {
        errorRes  = getErrorResponse(500, ""); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
    }

    size_t bCTnlPos = body.find("\r\n\r\n", cdnlPos + 2); // body content type new line (end) pos 

    string bodyContentType = body.substr(cdnlPos + 2, bCTnlPos - cdnlPos); // skip new line

    cout << "content len -> "  << contentLen << " bytes to upload" << endl;
    if (body.find(endBoundary) != string::npos)
    {
        cout << "body data is small enough, writing directly to file" << endl;
        size_t bodyEndPos = body.find(endBoundary);
        string bodyData = body.substr(bCTnlPos + 4, bodyEndPos - bCTnlPos - 4); // skip new line
        // cout << " [[" <<  bodyData << "]]" << endl;
        write(fd, bodyData.c_str(), bodyData.size());
    }
    else
    {
        cout << "body data is too large, reading from socket" << endl;
        // cout << "body data is too large, reading from socket" << endl;
        size_t headersPlusDataRead = req.body.size();  // How much we already have read (headers + body headers + maybe some body data)
        size_t totalDataNeeded = contentLen;  // total data we need to read got it from -> Content-Length header
        size_t remainingToRead = totalDataNeeded - headersPlusDataRead; // the remaining data to read, will be reading until i reach it
        // cout << "Content-Length: " << contentLen << endl;
        // cout << "Already read: " << headersPlusDataRead << endl;
        // cout << "Still need to read: " << remainingToRead << endl;
        string remaningStr = body.substr(bCTnlPos + 4);
        write(fd, remaningStr.c_str(), remaningStr.size());

        char buff[BUFFSIZE + 1];
        size_t totalRead = 0;
        
        while (totalRead < remainingToRead)
        {
            size_t toRead = min((size_t)BUFFSIZE, remainingToRead - totalRead);
            size_t bytesRead = recv(req.cfd, buff, toRead, 0);
            
            if (bytesRead == 0) {
                cout << "Client closed connection" << endl;
                break;
            }
            if (bytesRead == -1) {
                close(fd);
                throw NetworkException("recv failed during upload", 500);
            }
            write(fd, buff, bytesRead);
            totalRead += bytesRead;
            cout << "Read " << bytesRead << " bytes, total: " << totalRead / 1000 << "/" << remainingToRead / 1000 << endl;
        }
        cout << "Upload completed! Total bytes written: " << totalRead << endl;
    }
    close(fd);
    // success

    cout << "File uploaded successfully to " << newFile << endl;
    auth->redirectToPage(req.cfd, "./www/auth/profile.html" , 200); // redirect to login page with success message
}
