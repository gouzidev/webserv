#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"
#include "../../../includes/Exceptions.hpp"
#include <string>



void parseChunk(Request &req)
{

}

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

string WebServ::
getOriginalFileName(Request &req, string fileNameWithUserId, unsigned int &userIdAssociated)
{
    userIdAssociated = 0;
    short namePrefixSize = MAX_USERID_DIGITS + 1; // size of ([MAX_USERID_DIGITS] + ['_'])

    cout << "name man3ref xno " << namePrefixSize << endl;

    if (fileNameWithUserId.size() < namePrefixSize + 1) // 1 -> at least '_' and a character in the filename with the prefix
        throw RequestException("Invalid file name format", 400, req);
    cout << fileNameWithUserId << "where is problem " << fileNameWithUserId[namePrefixSize] << endl;
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

bool checkIfChunked(Request &req)
{
    if (exists(req.headers, "transfer-encoding"))


    {
        string transferEncoding = req.headers["transfer-encoding"];
        return transferEncoding == "chunked";
    }
    return false;
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

void handleChunkedUpload(Request &req, string bodyBoundary, ServerNode &serv, LocationNode &locationNode)
{

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


enum ParserState
{
    pSearchingForBoundary,
    pParsingHeaders,
    pStreamingBody,
    pDone
};

enum MultipartState
{
    pMultipartBoundary,
    pMultipartHeaders,
    pMultipartBody,
    pMultipartDone
};

// States for the de-chunker
enum ChunkedState
{
    pChunkSize,
    pChunkData,
    pChunkEnd
};

// Your helper function to get the filename
std::string getFilenameFromHeaders(const std::string& headers)
{
    size_t pos;
    size_t end_pos;
    struct FileUploadData fileData;
    fileData.filename = "";
    fileData.name = "";
    pos = headers.find("filename=\"");
    if (pos == std::string::npos)
        return fileData.filename;
    pos += 10;
    end_pos = headers.find("\"", pos);
    if (end_pos != std::string::npos)
        fileData.filename = headers.substr(pos, end_pos - pos);
    

    pos = headers.find("name=\"");
    if (pos == std::string::npos)
        return fileData.filename;
    pos += 6;
    end_pos = headers.find("\"", pos);
    if (end_pos != std::string::npos)
        fileData.name = headers.substr(pos, end_pos - pos);
    return fileData.filename;
    
}

struct FileUploadData getContentDataFromHeaders(const std::string& headers)
{
    size_t pos;
    size_t end_pos;
    struct FileUploadData fileData;
    fileData.filename = "";
    fileData.name = "";
    pos = headers.find("name=\"");
    if (pos != std::string::npos)
    {
        pos += 6;
        end_pos = headers.find("\"", pos);
        if (end_pos != std::string::npos)
            fileData.name = headers.substr(pos, end_pos - pos);
    }


    pos = headers.find("filename=\"");
    if (pos != std::string::npos)
    {
        pos += 10;
        end_pos = headers.find("\"", pos);
        if (end_pos != std::string::npos)
            fileData.filename = headers.substr(pos, end_pos - pos);
    }
    
    return fileData;
    
}


// The single, unified upload handler function
void WebServ::handleUpload(Request &req, ServerNode &serv, LocationNode &locationNode)
{

    string sessionKey = req.extractSessionId();
    if (!auth->isLoggedIn(sessionKey))
    {
        sendErrPageToClient(req.cfd, 401, serv);
        return ;
    }
    User &LoggedUser = auth->sessions.find(sessionKey)->second.getUser();

    string formDataDiv = "";

    // === 1. UNIFIED SETUP ===
    // this section prepares all variables needed for both normal and chunked uploads.

    // first, determine the transfer type. This flag controls which logic path we take.
    bool isChunked = checkIfChunked(req);

    // -- State for the Multipart Parser --
    // this state machine handles the actual file content (boundaries, headers, body).
    MultipartState multipartState = pMultipartBoundary;
    int filefd = -1;
    std::string filename;
    std::string name;
    // -- State for the De-Chunker (only used if isChunked is true) --
    // this state machine handles the chunked encoding itself (e.g., "4\r\nWiki\r\n").
    ChunkedState chunkedState = pChunkSize;
    size_t remaining_in_chunk = 0;
    
    // -- Buffers --
    // `socket_chunk` holds the raw data exactly as it comes from the network.
    std::string socket_chunk = req.body;
    // `multipart_chunk` holds the clean, continuous data after de-chunking.
    // for normal requests, data moves directly from socket_chunk to multipart_chunk.
    std::string multipart_chunk;

    // -- Network variables --
    char socket_read_buffer[BUFFSIZE];
    ssize_t bytesRead;

    std::string &contentType = req.headers["content-type"];
    size_t headerBoundaryPos = contentType.find("boundary=");
    if (headerBoundaryPos == std::string::npos)
    {
        sendErrPageToClient(req.cfd, 400, serv);
        return;
    }
    
    // -- boundary strings for the multipart parser --
    std::string rawBoundary = contentType.substr(headerBoundaryPos + 9);
    std::string boundary_marker = "\r\n--" + rawBoundary;
    std::string end_boundary_marker = boundary_marker + "--\r\n"; // recently added \r\n bcause when i debugged, i found out the data ends with it.
    std::string first_boundary_marker = "--" + rawBoundary;

    // === 2. CORRECTED UNIFIED MAIN LOOP ===
    // this new loop structure is the main fix.
    // it will run forever until we explicitly `break` out of it when parsing is
    // completely finished or a fatal error occurs.
    while (true)
    {
        // --- LAYER 1: De-Chunker ---
        // fills the multipart_chunk with clean data
        if (isChunked)
        {
            bool chunk_progress = true;
            while (chunk_progress && chunkedState != pChunkEnd)
            {
                chunk_progress = false;
                if (chunkedState == pChunkSize)
                {
                    size_t pos = socket_chunk.find("\r\n");
                    if (pos != std::string::npos)
                    {
                        std::string hex_size = socket_chunk.substr(0, pos);
                        if (hex_size.empty()) { break; }
                        
                        // **FIX**: Correctly handle the specific exception your function throws.
                        try {
                            remaining_in_chunk = stringToHexLong(hex_size, req);
                        } catch (const RequestException& e) {
                            // Handle malformed hex value by stopping the process.
                            multipartState = pMultipartDone;
                            break;
                        }

                        socket_chunk.erase(0, pos + 2);

                        if (remaining_in_chunk == 0)
                        {
                            chunkedState = pChunkEnd;
                        }
                        else
                        {
                            chunkedState = pChunkData;
                        }
                        chunk_progress = true;
                    }
                }
                else if (chunkedState == pChunkData)
                {
                    // **FIX**: We must have the data AND the trailing "\r\n".
                    if (socket_chunk.length() >= remaining_in_chunk + 2)
                    {
                        multipart_chunk.append(socket_chunk, 0, remaining_in_chunk);
                        socket_chunk.erase(0, remaining_in_chunk + 2);
                        chunkedState = pChunkSize;
                        chunk_progress = true;
                    }
                }
            }
        }
        else
        {
            multipart_chunk.append(socket_chunk);
            socket_chunk.clear();
        }

        // --- LAYER 2: MULTIPART CONTENT PARSING ---
        // this layer operates on the clean `multipart_chunk`.
        bool multipart_progress = true;
        while (multipart_progress && multipartState != pMultipartDone)
        {
            multipart_progress = false;

            if (multipartState == pMultipartBoundary)
            {
                size_t boundary_pos = multipart_chunk.find(first_boundary_marker);
                if (boundary_pos != 0)
                {
                    boundary_pos = multipart_chunk.find(boundary_marker);
                }
                if (boundary_pos != std::string::npos)
                {
                    size_t line_end_pos = multipart_chunk.find("\r\n", boundary_pos);
                    if (line_end_pos != std::string::npos)
                    {
                        multipart_chunk.erase(0, line_end_pos + 2);
                        multipartState = pMultipartHeaders;
                        multipart_progress = true;
                    }
                }
            }
            else if (multipartState == pMultipartHeaders)
            {
                size_t headers_end_pos = multipart_chunk.find("\r\n\r\n");
                if (headers_end_pos != std::string::npos)
                {
                    std::string headers = multipart_chunk.substr(0, headers_end_pos);
                    struct FileUploadData fileData = getContentDataFromHeaders(headers);
                    std::string filename = fileData.filename;
                    cout << "File name: " << fileData.filename << ", Name: " << fileData.name << ", after processing -> " << filename << endl;
                    if (!filename.empty())
                    {
                        filename = getFileNameWithUserId(req, LoggedUser.getId(), filename);
                        std::string filepath = locationNode.uploadDir + "/" + filename;
                        filefd = open(filepath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
                    }
                    else
                    {
                        name = fileData.name;
                    }
                    multipart_chunk.erase(0, headers_end_pos + 4);

                    if (multipart_chunk == end_boundary_marker)
                        multipartState = pMultipartDone;
                    else
                        multipartState = pMultipartBody;
                    multipartState = pMultipartBody;
                    multipart_progress = true;
                }
            }
            else if (multipartState == pMultipartBody)
            {
                size_t end_boundary_pos = multipart_chunk.find(end_boundary_marker);
                size_t next_boundary_pos = multipart_chunk.find(boundary_marker);

                if (next_boundary_pos != std::string::npos)
                {
                    if (filefd != -1)
                    {
                        write(filefd, multipart_chunk.c_str(), next_boundary_pos);
                        close(filefd);
                        filefd = -1;
                    }
                    else 
                    {
                        string value = multipart_chunk.substr(0, next_boundary_pos);
                        formDataDiv += getDataStrInDiv(name, value);
                    }
                    multipart_chunk.erase(0, next_boundary_pos);
                    if (multipart_chunk == end_boundary_marker)
                        multipartState = pMultipartDone;
                    else
                        multipartState = pMultipartBoundary;
                    multipart_progress = true;
                }
                
                else if (end_boundary_pos != std::string::npos)
                {
                    if (filefd != -1)
                    {
                        write(filefd, multipart_chunk.c_str(), end_boundary_pos);
                        close(filefd);
                        filefd = -1;
                    }
                    multipartState = pMultipartDone;
                    multipart_progress = true;
                }
                else
                {
                    size_t write_size = 0;
                    if (multipart_chunk.length() > boundary_marker.length())
                    {
                        write_size = multipart_chunk.length() - boundary_marker.length();
                    }
                    if (filefd != -1 && write_size > 0)
                    {
                        write(filefd, multipart_chunk.c_str(), write_size);
                        multipart_chunk.erase(0, write_size);
                    }
                }
            }
        }

        // if we are done, we can exit the main loop.
        if (multipartState == pMultipartDone || (isChunked && chunkedState == pChunkEnd))
        {
            break;
        }

        // if we reach here, it means the parsers need more data.
        bytesRead = recv(req.cfd, socket_read_buffer, BUFFSIZE, 0);
        if (bytesRead > 0)
        {
            socket_chunk.append(socket_read_buffer, bytesRead);
        }
        else
        {
            // if recv returns 0 or -1, the client has disconnected.
            // break the loop and let the final check handle the error.
            break;
        }
    }
    
    // === 3. FINAL CHECK & CLEANUP ===
    if (multipartState != pMultipartDone)
    {
        std::cerr << "Error: Upload finished unexpectedly.\n";
        if (filefd != -1)
        {
            close(filefd);
        }
    }
    else
    {
        std::cout << "Success: Upload finished.\n";
        // auth->redirectToPage(req.cfd, "./www/auth/profile.html", 200); // Your success logic
    }

    map <string, string> data;
    data["data"] =  formDataDiv;
    string page = dynamicRender("./www/auth/form.html", data);
    req.resp.setStatusLine("HTTP/1.1 200 OK\r\n");
    makeResponse(req, page);
}