#include "../../../includes/webserv.hpp"
#include "../../../includes/Debugger.hpp"
#include "../../../includes/Exceptions.hpp"

void WebServ::handleLogin(Client &client)
{
    Request &req = client.request;
    ServerNode &serv = req.serv;
    string errorRes;

    map < string , string > queryParams;

    string body = client.requestBuff;

    if (!exists(req.headers, "content-type"))
    {
        cerr << "send a host and content-type mf" << endl;
        throw HttpException(400, client);
    }
    if (!strHas(body, "email=") || !strHas(body, "password="))
    {
        cerr << "please provide an email and password" << endl;
        throw HttpException(400, client);
    }

    urlFormParser(body, queryParams);

    if (!exists(queryParams, string("email")) || !exists(queryParams, string("password")))
    {
        cerr << "email or password not found in query params" << endl;
        throw HttpException(400, client);
    }
    string email = queryParams["email"];
    string password = queryParams["password"];

    auth->login(client, email, password);
}

void WebServ::handleFormData(Client &client)
{
    Request &req = client.request;
    ServerNode &serv = req.serv;
    string errorRes;
    string dataDivStr = "<div>";
    map < string , string > queryParams;

    string body = client.requestBuff;

    if (!exists(req.headers, "content-type"))
    {
        cerr << "send a host and content-type mf" << endl;
        throw HttpException(400, client);
    }

    urlFormParser(body, queryParams);


    map < string , string >::iterator it;

    it = queryParams.begin();

    while (it != queryParams.end())
    {
        if (!it->second.empty())
            dataDivStr += "<li>" + it->first + " -> " + it->second + "</li>";
        else
            dataDivStr += "<li><i>" + it->first + "</i></li>";
        it++;
    }
    dataDivStr += "</div>";

    queryParams["data"] = dataDivStr;
    string page = dynamicRender("/home/sgouzi/webserv/www/auth/form.html", queryParams);

    string response;
    response += "HTTP/1.1 " + ushortToStr(201) + " " + getStatusMessage(201) + " \r\n";
    response +=  "Content-Type: text/html\r\n";
    response +=  "Content-Length: " + ushortToStr(page.size()) + "\r\n\r\n";
    response += page;

    client.responseBuff = response;
    
}

string WebServ::getDataStrInDiv(string &name, string &value)
{
    string errorRes;
    string dataDivStr = "<div>";
    dataDivStr += "<li>" + name + " ---> " + value + "</li>";
    dataDivStr += "</div>";

    
    return dataDivStr;
}


// // The single, unified upload handler function
// void WebServ::handleUpload(Client &client, LocationNode &locationNode)
// {
//     Request &req = client.request;
//     ServerNode &serv = req.serv;

//     long long clientMaxUploadSize = serv.clientMaxBodySize * 1024 * 1024;
//     string sessionKey = req.extractSessionId();
//     if (!auth->isLoggedIn(sessionKey))
//     {
//         cout << "unauthorized upload attempt" << endl;
//         throw HttpException(401, client);
//     }
//     User &LoggedUser = auth->sessions.find(sessionKey)->second.getUser();

//     long contentLen = client.request.contentLen;
//     if (contentLen > clientMaxUploadSize)
//     {
//         cout << "upload size exceeds limit" << endl;
//         throw HttpException(413, client);
//     }
//     string formDataDiv = "";

//     // === 1. UNIFIED SETUP ===
//     // this section prepares all variables needed for both normal and chunked uploads.

//     // first, determine the transfer type. This flag controls which logic path we take.
//     bool isChunked = req.isChunked;

//     // -- State for the Multipart Parser --
//     // this state machine handles the actual file content (boundaries, headers, body).
//     MultipartState multipartState = pMultipartBoundary;
//     int filefd = -1;
//     std::string filename;
//     std::string name;


//     // byte tracking
//     ssize_t bytesRead;
//     ssize_t bytesTotal;

//     // -- State for the De-Chunker (only used if isChunked is true) --
//     // this state machine handles the chunked encoding itself (e.g., "4\r\nWiki\r\n").
//     ChunkedState chunkedState = pChunkSize;
//     size_t remaining_in_chunk = 0;
    
//     // -- Buffers --
//     // `socket_chunk` holds the raw data exactly as it comes from the network.
//     std::string socket_chunk = req.body;
//     bytesTotal = req.bodyLen;
//     // `multipart_chunk` holds the clean, continuous data after de-chunking.
//     // for normal requests, data moves directly from socket_chunk to multipart_chunk.
//     std::string multipart_chunk;

//     // -- Network variables --
//     char socket_read_buffer[BUFFSIZE];


//     std::string &contentType = req.headers["content-type"];
//     size_t headerBoundaryPos = contentType.find("boundary=");
//     if (headerBoundaryPos == std::string::npos)
//     {
//         cout << "boundary not found in content-type header" << endl;
//         throw HttpException(400, client);
//     }
    
//     // -- boundary strings for the multipart parser --
//     std::string rawBoundary = contentType.substr(headerBoundaryPos + 9);
//     std::string boundary_marker = "\r\n--" + rawBoundary;
//     std::string end_boundary_marker = boundary_marker + "--\r\n"; // recently added \r\n bcause when i debugged, i found out the data ends with it.
//     std::string first_boundary_marker = "--" + rawBoundary;

//     // === 2. CORRECTED UNIFIED MAIN LOOP ===

//         // --- LAYER 1: De-Chunker ---
//         // fills the multipart_chunk with clean data
//         if (isChunked)
//         {
//             bool chunk_progress = true;
//             while (chunk_progress && chunkedState != pChunkEnd)
//             {
//                 chunk_progress = false;
//                 if (chunkedState == pChunkSize)
//                 {
//                     size_t pos = socket_chunk.find("\r\n");
//                     if (pos != std::string::npos)
//                     {
//                         std::string hex_size = socket_chunk.substr(0, pos);
//                         if (hex_size.empty()) { break; }
                        
//                         // **FIX**: Correctly handle the specific exception your function throws.
//                         try {
//                             remaining_in_chunk = stringToHexLong(hex_size, req);
//                         } catch (const RequestException& e) {
//                             // Handle malformed hex value by stopping the process.
//                             multipartState = pMultipartDone;
//                             break;
//                         }

//                         socket_chunk.erase(0, pos + 2);

//                         if (remaining_in_chunk == 0)
//                         {
//                             chunkedState = pChunkEnd;
//                         }
//                         else
//                         {
//                             chunkedState = pChunkData;
//                         }
//                         chunk_progress = true;
//                     }
//                 }
//                 else if (chunkedState == pChunkData)
//                 {
//                     // **FIX**: We must have the data AND the trailing "\r\n".
//                     if (socket_chunk.length() >= remaining_in_chunk + 2)
//                     {
//                         multipart_chunk.append(socket_chunk, 0, remaining_in_chunk);
//                         socket_chunk.erase(0, remaining_in_chunk + 2);
//                         chunkedState = pChunkSize;
//                         chunk_progress = true;
//                     }
//                 }
//             }
//         }
//         else
//         {
//             multipart_chunk.append(socket_chunk);
//             socket_chunk.clear();
//         }

//         // --- LAYER 2: MULTIPART CONTENT PARSING ---
//         // this layer operates on the clean `multipart_chunk`.
//         bool multipart_progress = true;
//         while (multipart_progress && multipartState != pMultipartDone)
//         {
//             multipart_progress = false;

//             if (multipartState == pMultipartBoundary)
//             {
//                 size_t boundary_pos = multipart_chunk.find(first_boundary_marker);
//                 if (boundary_pos != 0)
//                 {
//                     boundary_pos = multipart_chunk.find(boundary_marker);
//                 }
//                 if (boundary_pos != std::string::npos)
//                 {
//                     size_t line_end_pos = multipart_chunk.find("\r\n", boundary_pos);
//                     if (line_end_pos != std::string::npos)
//                     {
//                         multipart_chunk.erase(0, line_end_pos + 2);
//                         multipartState = pMultipartHeaders;
//                         multipart_progress = true;
//                     }
//                 }
//             }
//             else if (multipartState == pMultipartHeaders)
//             {
//                 size_t headers_end_pos = multipart_chunk.find("\r\n\r\n");
//                 if (headers_end_pos != std::string::npos)
//                 {
//                     std::string headers = multipart_chunk.substr(0, headers_end_pos);
//                     struct FileUploadData fileData = getContentDataFromHeaders(headers);
//                     std::string filename = fileData.filename;
//                     if (!filename.empty())
//                     {
//                         filename = getFileNameWithUserId(req, LoggedUser.getId(), filename);
//                         std::string filepath = locationNode.uploadDir + "/" + filename;
//                         filefd = open(filepath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
//                     }
//                     else
//                     {
//                         name = fileData.name;
//                     }
//                     multipart_chunk.erase(0, headers_end_pos + 4);

//                     if (multipart_chunk == end_boundary_marker)
//                         multipartState = pMultipartDone;
//                     else
//                         multipartState = pMultipartBody;
//                     multipartState = pMultipartBody;
//                     multipart_progress = true;
//                 }
//             }
//             else if (multipartState == pMultipartBody)
//             {
//                 size_t end_boundary_pos = multipart_chunk.find(end_boundary_marker);
//                 size_t next_boundary_pos = multipart_chunk.find(boundary_marker);

//                 if (next_boundary_pos != std::string::npos)
//                 {
//                     if (filefd != -1)
//                     {
//                         write(filefd, multipart_chunk.c_str(), next_boundary_pos);
//                         close(filefd);
//                         filefd = -1;
//                     }
//                     else 
//                     {
//                         string value = multipart_chunk.substr(0, next_boundary_pos);
//                         formDataDiv += getDataStrInDiv(name, value);
//                     }
//                     multipart_chunk.erase(0, next_boundary_pos);
//                     if (multipart_chunk == end_boundary_marker)
//                         multipartState = pMultipartDone;
//                     else
//                         multipartState = pMultipartBoundary;
//                     multipart_progress = true;
//                 }
                
//                 else if (end_boundary_pos != std::string::npos)
//                 {
//                     if (filefd != -1)
//                     {
//                         write(filefd, multipart_chunk.c_str(), end_boundary_pos);
//                         close(filefd);
//                         filefd = -1;
//                     }
//                     multipartState = pMultipartDone;
//                     multipart_progress = true;
//                 }
//                 else
//                 {
//                     size_t write_size = 0;
//                     if (multipart_chunk.length() > boundary_marker.length())
//                     {
//                         write_size = multipart_chunk.length() - boundary_marker.length();
//                     }
//                     if (filefd != -1 && write_size > 0)
//                     {
//                         write(filefd, multipart_chunk.c_str(), write_size);
//                         multipart_chunk.erase(0, write_size);
//                     }
//                 }
//             }
//         }

    
//     // === 3. FINAL CHECK & CLEANUP ===
//     if (multipartState != pMultipartDone)
//     {
//         std::cerr << "Error: Upload finished unexpectedly.\n";
//         if (filefd != -1)
//         {
//             close(filefd);
//         }
//     }
//     else
//     {
//         std::cout << "Success: Upload finished.\n";
//         // auth->redirectToPage(req.cfd, "./www/auth/profile.html", 200); // Your success logic
//     }

//     map <string, string> data;
//     data["data"] =  formDataDiv;
//     string page = dynamicRender("./www/auth/form.html", data);
//     req.resp.setStatusLine("HTTP/1.1 200 OK\r\n");
//     makeResponse(req, page);
// }

void WebServ::handleSignup(Client &client)
{
    Request &req = client.request;
    ServerNode &serv = req.serv;
    string errorRes;

    string body = client.requestBuff;

    if (!exists(req.headers, "content-type"))
    {
        cerr << "send a host and content-type mf" << endl;
        throw HttpException(400, client);
    }

    if (!strHas(body, "firstName=") || !strHas(body, "lastName=")
        || !strHas(body, "email=") || !strHas(body, "password="))
    {
        cerr << "please provide firstName, lastName, email and password" << endl;
        throw HttpException(400, client);
    }
    map < string , string > queryParams;
    urlFormParser(body, queryParams);
    if (!exists(queryParams, string("firstName")) || !exists(queryParams, string("lastName"))
        || !exists(queryParams, string("email")) || !exists(queryParams, string("password"))
        )
    {
        cerr << "firstName, lastName, email or password not found in query params" << endl;
        throw HttpException(400, client);
    }
    string userName;
    if (exists(queryParams, "userName"))
        userName = queryParams["userName"];
    string fName = queryParams["firstName"];
    string lName = queryParams["lastName"];
    string email = queryParams["email"];
    string password = queryParams["password"];

    auth->signup(client, fName, userName, lName, email, password);
}

void WebServ::handleLogout(Client &client)
{
    Request &req = client.request;
    ServerNode &serv = req.serv;
    cout << "handling logout" << endl;
    string sessionKey = req.getSessionKey();
    auth->logout(client, sessionKey);
}

// long long extractContentLen(Client &client)
// {
//     Request &req = client.request;
//     ServerNode &serv = req.serv;
//     if (!exists(req.headers, "content-length"))
//     {
//         cerr << "could not find 'content-length' header" << endl;
//         throw HttpException(400, client);
//     }
//     string contentLenStr = req.headers.find("content-length")->second;
//     contentLenStr = trimWSpaces(contentLenStr);
//     if (!strAllDigit(contentLenStr))
//     {
//         cerr << "content-length is not a number: " << contentLenStr << endl;
//         throw  HttpException(400, client);
//     }

//     istringstream contentLenStream (contentLenStr);
//     if (contentLenStream.fail())
//     {
//         cerr << "content-length stream failed" << endl;
//         throw HttpException(400, client);
//     }
//     long long contentLen;
//     contentLenStream >> contentLen;
//     return contentLen;
// }

void WebServ::postMethode(Client &client)
{
    Request &req = client.request;
    string &buff = client.requestBuff;
    map <string, string> &headers = req.headers;
    ServerNode &serv = req.serv;
    string locationTarget = getLocation(req, serv); // will get "/" if the location is not in the server--
    if (locationTarget == "") // doesnt exist
    {
        cout << "location not found in server node for target -> " << req.getResource() << endl;
        throw HttpException(404, client);
    }

    string errorRes;
    LocationNode locationNode = serv.locationDict.find(locationTarget)->second;
    if (!exists(locationNode.methods, string("POST"))) // methods are stored in upper case
        throw HttpException(405, client);
    vector <string> startLine = req.getStartLine();
    string &location = req.getResource();

    if (!exists(req.headers, "content-length"))
    {
        // sendErrPageToClient(req.cfd, 400, serv);
        cout << "could not find 'content-length'" << endl;
        throw HttpException(400, client);
    }
    long long contentLen = req.contentLen; // stored in MB
    if (contentLen <= 0 && locationNode.needContentLen)
    {
        // sendErrPageToClient(req.cfd, 400, serv);
        cout << "content length is not valid" << endl;
        throw HttpException(400, client);
    }

    if (!exists(req.headers, "content-type"))
    {
        // sendErrPageToClient(req.cfd, 400, serv);
        cout << "could not find 'content-type'" << endl;
        throw HttpException(400, client);
    }
    string contentType = headers.find("content-type")->second;
    string rootFolder = serv.root;

    if (locationTarget == "") // doesnt exist
    {
        cout << "location not found in server node for target -> " << req.getResource() << endl;
        throw HttpException(404, client);
    }

    cout << "location target is " << locationTarget << endl;

    if (locationNode.isProtected)
    {
        string sessionKey = req.extractSessionId();
        if (!auth->isLoggedIn(sessionKey))
        {
            cout << "un auth" << endl;
            throw HttpException(401, client);
        }
    }


    cout << "content type is " << contentType << endl;
    if (contentType == "application/x-www-form-urlencoded") // handle form post request
    {
        if (locationTarget == "/login")
            handleLogin(client);
        else if (locationTarget == "/signup")
            handleSignup(client);
        else if (locationTarget == "/logout")
            handleLogout(client);
        else
            handleFormData(client);
    }   
    else if (startsWith(contentType, "multipart/form-data; boundary=")) // handle file upload
    {
        cout << "handling file upload" << endl;
        handleUpload(client, locationNode);
    }
    else
    {
        cout << "content type is not supported for post request" << endl;
        const char *successResponse =
        "HTTP/1.1 404 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 12\r\n"
        "\r\n"
        "Successfull!";
        send(req.cfd, successResponse, strlen(successResponse), 0);

    }
    client.requestBuff.clear();
    client.clientState = SENDING_CHUNKS;
}