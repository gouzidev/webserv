#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"
#include "../../../includes/Exceptions.hpp"

bool verifyUpload(Request &req, ServerNode &servNode, LocationNode &locationNode, size_t &boundaryPos)
{
    string errorRes;
    if (locationNode.uploadDir == "")
    {
        errorRes  = getErrorResponse(405, ""); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return false;
        // error (no upload path provided)
    }
    string &contentType = req.headers["content-type"];
    boundaryPos = contentType.find("boundary=");
    if (boundaryPos == string::npos)
    {
        sendErrPageToClient(req.cfd, 400, servNode);
        return false;
    }
    return true;
}

struct fileUploadData
{
    string name;
    string filename;
};

struct fileUploadData getUploadData(string &contentDisposition)
{
    struct fileUploadData data;
    size_t i = 0;
 
    struct fileUploadData fileData;
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
            data.name = contentDisposition.substr(firstQuote + 1, lastQuote - firstQuote - 1);
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
            data.filename = contentDisposition.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            i = j;
            continue;
        }
        i++;
    }
    return data;
}


bool handleUploadData(Request &req, size_t currBodyPos, size_t bodyBoundaryPos, size_t endBoundarySize, LocationNode &locationNode)
{
    string errorRes;
    string bodyData;
    string body = req.body;
    size_t cdPos = body.find("Content-Disposition:", currBodyPos); // content disposition (start) pos
    size_t cdEndPos = body.find("\r\n", cdPos);  // content disposition new line (end) pos (and start of body content type after ..)
    string contentDisposition = body.substr(cdPos, cdEndPos - cdPos);
    struct fileUploadData fileData = getUploadData(contentDisposition);

    size_t bctEndPos = body.find("\r\n\r\n", cdEndPos + 2); // body content type new line (end) pos 


    string bodyContentType = body.substr(cdEndPos + 2, bctEndPos - cdEndPos);

    if (endBoundarySize != string::npos)
    {
        bodyData = body.substr(currBodyPos, bodyBoundaryPos - currBodyPos- endBoundarySize);
        cout << "bodyData: " << bodyData << endl;
        cout << "end boundary size: " << endBoundarySize << endl;
    }
    else
        bodyData = body.substr(currBodyPos, bodyBoundaryPos - currBodyPos);
    string path = locationNode.uploadDir;
    string newFile = path + "/" + fileData.filename;

    int fd = open(newFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1)
    {
        errorRes  = getErrorResponse(500, ""); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return false;
    }
    
    write(fd, bodyData.c_str(), bodyData.size());
    close(fd);
    return true;
}

void WebServ::handleUplaod(Request &req, long contentLen, ServerNode &servNode, LocationNode &locationNode)
{


    // init some vars
    string errorRes;


    // GET THE BOUNDARY AND VERIFY
    string &contentType = req.headers["content-type"];
    size_t headerBoundaryPos;
    if (verifyUpload(req, servNode, locationNode, headerBoundaryPos) == false)
        return ;

    string headerBoundary = contentType.substr(headerBoundaryPos + 9);
    string headerStartBoundary = "--" + headerBoundary;
    string bodyBoundary = headerStartBoundary + "\r\n";
    string body = req.body;

    if (body.substr(0, bodyBoundary.size()) != bodyBoundary)
        return (sendErrPageToClient(req.cfd, 400, servNode));

    string endBoundary = "\r\n" + headerStartBoundary + "--";

    size_t endBoundaryPos = body.find(endBoundary);


    if (endBoundaryPos != string::npos) // small body
    {
        string bodyData;
        size_t currBodyPos = bodyBoundary.size(); // start the search after the Boundary
        size_t bodyBoundaryPos = body.find(bodyBoundary, currBodyPos);
        while (bodyBoundaryPos != string::npos) // while we still have multiple uploads
        {
            if (handleUploadData(req, currBodyPos, bodyBoundaryPos, string::npos, locationNode) == false)
                return ;
            currBodyPos = bodyBoundaryPos + bodyBoundary.size(); // including \r\n \r\n
            bodyBoundaryPos = body.find(bodyBoundary, currBodyPos);
        }
        if (handleUploadData(req, currBodyPos, bodyBoundaryPos, endBoundary.size(), locationNode) == false)
            return ;
    }


    // success
    auth->redirectToPage(req.cfd, "./www/auth/profile.html" , 200); // redirect to login page with success message
}
