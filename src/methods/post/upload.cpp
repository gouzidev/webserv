#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"
#include "../../../includes/Exceptions.hpp"

struct FileUploadData
{
    string name;
    string filename;
};

string WebServ::getFileNameWithUserId(Request &req, unsigned int userId, string originalName)
{
    stringstream ss;
    
    ss << userId;
    string userIdStr = ss.str();
    int i = userIdStr.size();
    while (i++ < MAX_USERID_DIGITS)
        userIdStr.insert(0, "0");
    string resName = userIdStr + "_" + originalName;
    return resName;
}

string WebServ::getOriginalFileName(Request &req, string fileNameWithUserId, unsigned int &userIdAssociated)
{
    userIdAssociated = 0;
    short namePrefixSize = MAX_USERID_DIGITS + 1; // size of ([MAX_USERID_DIGITS] + ['_'])

    if (fileNameWithUserId.size() < namePrefixSize + 1) // 1 -> at least '_' and a character in the filename with the prefix
        throw RequestException("Invalid file name format", 400, req);
    if (fileNameWithUserId[MAX_USERID_DIGITS] != '_')
        throw RequestException("Invalid file name format", 400, req);
    string userIdFromFileName = fileNameWithUserId.substr(0, MAX_USERID_DIGITS);

    userIdAssociated = atoi(userIdFromFileName.c_str());
    string originalName = fileNameWithUserId.substr(namePrefixSize);
    return originalName;
}

struct FileUploadData getUploadData(string &contentDisposition)
{
    struct FileUploadData data;
    ssize_t i = 0;
 
    struct FileUploadData fileData;
    while (i < contentDisposition.size())


    {
        if (contentDisposition.substr(i, 5) == "name=")
    
    
        {
            ssize_t firstQuote = -1;
            ssize_t lastQuote = -1;
            ssize_t j = i + 5;
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
            ssize_t firstQuote = -1;
            ssize_t lastQuote = -1;
            ssize_t j = i + 9;
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



string getFileNameMoveIdx(size_t readPos, string &body, size_t &idx)
{
    size_t cdPos = body.find("Content-Disposition:", readPos); // content disposition (start) pos
    size_t cdEndPos = body.find("\r\n", cdPos);  // content disposition new line (end) pos (and start of body content type after ..)
    string contentDisposition = body.substr(cdPos, cdEndPos - cdPos);


    size_t ctPos = body.find("Content-Type:", cdEndPos);
    size_t ctEndPos = body.find("\r\n", ctPos);  // content type new line (end) pos

    struct FileUploadData fileData = getUploadData(contentDisposition);

    idx = ctEndPos + 4;
    return fileData.filename;
}

int openCurrFile(Request &req, LocationNode &locationNode, string fileName)
{
    string errorRes;
    string path = locationNode.uploadDir;
    string newFile = path + "/" + fileName;

    int fd = open(newFile.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd == -1)


    {
        errorRes  = getErrorResponse(500, ""); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return -1;
    }
    return fd;
}

void writeChunk(Request &req, LocationNode &locationNode, string chunk, size_t chunkSize, string fileName)
{
    string errorRes;
    string path = locationNode.uploadDir;
    string newFile = path + "/" + fileName;

    int fd = open(newFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1)


    {
        errorRes  = getErrorResponse(500, ""); // method not allowed 
        send(req.cfd, errorRes.c_str(), errorRes.length(), 0);
        return ;
    }
    write(fd, chunk.c_str(), chunkSize);
    close(fd);
}

bool verifyUpload(Request &req, ServerNode &serv, LocationNode &locationNode, size_t &boundaryPos)
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
        sendErrPageToClient(req.cfd, 400, serv);
        return false;
    }
    return true;
}




// Your helper function to get the filename
string getFilenameFromHeaders(const string& headers)
{
    size_t pos;
    size_t end_pos;
    struct FileUploadData fileData;
    fileData.filename = "";
    fileData.name = "";
    pos = headers.find("filename=\"");
    if (pos == string::npos)
        return fileData.filename;
    pos += 10;
    end_pos = headers.find("\"", pos);
    if (end_pos != string::npos)
        fileData.filename = headers.substr(pos, end_pos - pos);
    

    pos = headers.find("name=\"");
    if (pos == string::npos)
        return fileData.filename;
    pos += 6;
    end_pos = headers.find("\"", pos);
    if (end_pos != string::npos)
        fileData.name = headers.substr(pos, end_pos - pos);
    return fileData.filename;
    
}

struct FileUploadData getContentDataFromHeaders(const string& headers)
{
    size_t pos;
    size_t end_pos;
    struct FileUploadData fileData;
    fileData.filename = "";
    fileData.name = "";
    pos = headers.find("name=\"");
    if (pos != string::npos)
    {
        pos += 6;
        end_pos = headers.find("\"", pos);
        if (end_pos != string::npos)
            fileData.name = headers.substr(pos, end_pos - pos);
    }


    pos = headers.find("filename=\"");
    if (pos != string::npos)
    {
        pos += 10;
        end_pos = headers.find("\"", pos);
        if (end_pos != string::npos)
            fileData.filename = headers.substr(pos, end_pos - pos);
    }
    
    return fileData;
    
}



// The single, unified upload handler function
void WebServ::handleUpload(Client &client, LocationNode &locationNode)
{
    Request &req = client.request;
    ServerNode &serv = req.serv;

    long long clientMaxUploadSize = serv.clientMaxBodySize * 1024 * 1024;
    string sessionKey = req.extractSessionId();

    // if (!auth->isLoggedIn(sessionKey))
    // {
    //     cout << "unauthorized upload attempt" << endl;
    //     throw HttpException(401, client);
    // }

    long contentLen = client.request.contentLen;
    if (contentLen > clientMaxUploadSize)
    {
        cout << "upload size exceeds limit" << endl;
        throw HttpException(413, client);
    }
    // User &LoggedUser = auth->sessions.find(sessionKey)->second.getUser();

    string formDataDiv = "";
    UploadData *data;
    if (req.uploadData == NULL) // first time -> init it
    {
        req.uploadData = new UploadData(client);
    }
    else
    {
        req.uploadData->socket_chunk.append(client.requestBuff);
    }
    data = req.uploadData;


    // === 1. UNIFIED SETUP ===
    // this section prepares all variables needed for both normal and chunked uploads.

    // first, determine the transfer type. This flag controls which logic path we take.
    bool isChunked = data->isChunked; // done

  
    // --- LAYER 1: De-Chunker ---
    // fills the multipart_chunk with clean data
    if (isChunked)
    {
        bool chunk_progress = true; // discard
        while (chunk_progress && data->chunkedState != pChunkEnd)
        {
            chunk_progress = false;
            if (data->chunkedState == pChunkSize)
            {
                size_t pos = data->socket_chunk.find("\r\n");
                if (pos != string::npos)
                {
                    string hex_size = data->socket_chunk.substr(0, pos);
                    if (hex_size.empty()) { break; }
                    
                    // handle the specific exception your function throws.
                    try {
                        data->remaining_in_chunk = stringToHexLong(hex_size, req);
                    } catch (const RequestException& e) {
                        // Handle malformed hex value by stopping the process.
                        data->multipartState = pMultipartDone;
                        break;
                    }

                    data->socket_chunk.erase(0, pos + 2);

                    if (data->remaining_in_chunk == 0)
                    {
                        data->chunkedState = pChunkEnd;
                    }
                    else
                    {
                        data->chunkedState = pChunkData;
                    }
                    chunk_progress = true;
                }
            }
            else if (data->chunkedState == pChunkData)
            {
                // **FIX**: We must have the data AND the trailing "\r\n".
                if (data->socket_chunk.length() >= data->remaining_in_chunk + 2)
                {
                    data->multipart_chunk.append(data->socket_chunk, 0, data->remaining_in_chunk);
                    data->socket_chunk.erase(0, data->remaining_in_chunk + 2);
                    data->chunkedState = pChunkSize;
                    chunk_progress = true;
                }
            }
        }
    }
    else
    {
        data->multipart_chunk.append(data->socket_chunk);
        data->socket_chunk.clear();
    }

    // --- LAYER 2: MULTIPART CONTENT PARSING ---
    // this layer operates on the clean `data->multipart_chunk`.
    bool multipart_progress = true;
    while (multipart_progress && data->multipartState != pMultipartDone)
    {
        multipart_progress = false;

        if (data->multipartState == pMultipartBoundary)
        {
            size_t boundary_pos = data->multipart_chunk.find(data->first_boundary_marker);
            if (boundary_pos != 0)
            {
                boundary_pos = data->multipart_chunk.find(data->boundary_marker);
            }
            if (boundary_pos != string::npos)
            {
                size_t line_end_pos = data->multipart_chunk.find("\r\n", boundary_pos);
                if (line_end_pos != string::npos)
                {
                    data->multipart_chunk.erase(0, line_end_pos + 2);
                    data->multipartState = pMultipartHeaders;
                    multipart_progress = true;
                }
            }
        }
        else if (data->multipartState == pMultipartHeaders)
        {
            size_t headers_end_pos = data->multipart_chunk.find("\r\n\r\n");
            if (headers_end_pos != string::npos)
            {
                string headers = data->multipart_chunk.substr(0, headers_end_pos);
                struct FileUploadData fileData = getContentDataFromHeaders(headers);
                string filename = fileData.filename;
                if (!filename.empty())
                {
                    filename = getFileNameWithUserId(req, 5, filename);
                    // filename = getFileNameWithUserId(req, LoggedUser.getId(), filename);
                    string filepath = locationNode.uploadDir + "/" + filename;
                    data->filefd = open(filepath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
                }
                else
                {
                    data->name = fileData.name;
                }
                data->multipart_chunk.erase(0, headers_end_pos + 4);

                if (data->multipart_chunk == data->end_boundary_marker)
                    data->multipartState = pMultipartDone;
                else
                    data->multipartState = pMultipartBody;
                multipart_progress = true;
            }
        }
        else if (data->multipartState == pMultipartBody)
        {
            size_t end_boundary_pos = data->multipart_chunk.find(data->end_boundary_marker);
            size_t next_boundary_pos = data->multipart_chunk.find(data->boundary_marker);

            if (next_boundary_pos != string::npos && next_boundary_pos != end_boundary_pos)
            {
                if (data->filefd != -1)
                {
                    write(data->filefd, data->multipart_chunk.c_str(), next_boundary_pos);
                    close(data->filefd);
                    data->filefd = -1;
                }
                else 
                {
                    string value = data->multipart_chunk.substr(0, next_boundary_pos);
                    formDataDiv += getDataStrInDiv(data->name, value);
                }
                data->multipart_chunk.erase(0, next_boundary_pos);
                if (data->multipart_chunk == data->end_boundary_marker)
                    data->multipartState = pMultipartDone;
                else
                    data->multipartState = pMultipartBoundary;
                multipart_progress = true;
            }
            
            else if (end_boundary_pos != string::npos)
            {
                if (data->filefd != -1)
                {
                    write(data->filefd, data->multipart_chunk.c_str(), end_boundary_pos);
                    close(data->filefd);
                    data->filefd = -1;
                }
                data->multipartState = pMultipartDone;
                multipart_progress = true;
            }
            else
            {
                size_t write_size = data->multipart_chunk.length();
                // if (data->multipart_chunk.length() > data->boundary_marker.length())
                // {
                //     write_size = data->multipart_chunk.length() - data->boundary_marker.length();
                // }
                if (data->filefd != -1 && write_size > 0)
                {
                    write(data->filefd, data->multipart_chunk.c_str(), write_size);
                    data->multipart_chunk.erase(0, write_size);
                }
            }
        }
    }

    
    // === 3. FINAL CHECK & CLEANUP ===
    if (data->multipartState == pMultipartError)
    {
        client.clientState = WRITING_ERROR;
        std::cerr << "Error: Upload finished unexpectedly.\n";
        if (data->filefd != -1)
        {
            close(data->filefd);
        }
        delete req.uploadData;
        req.uploadData = NULL;
    }
    else if (data->multipartState == pMultipartDone) // means we are done.
    {
        client.clientState = WRITING_RESPONSE;
        if (data->filefd != -1)
        {
            close(data->filefd);
        }
        delete req.uploadData;
        req.uploadData = NULL;
        std::cout << "Success: Upload finished.\n";
        // auth->redirectToPage(req.cfd, "./www/auth/profile.html", 200); // Your success logic
    }
    else // more to read
    {
        data->socket_chunk = data->multipart_chunk;
        data->multipart_chunk.clear();
        client.requestBuff.clear();
    }
    

    // map <string, string> dataMap;
    // dataMap["data"] =  formDataDiv;
    // string page = dynamicRender("./www/auth/form.html", dataMap);
    // req.resp.setStatusLine("HTTP/1.1 200 OK\r\n");
    // makeResponse(req, page);
}