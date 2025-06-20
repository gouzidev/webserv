#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <regex>
#include <chrono>
#include <iomanip>

namespace fs = std::filesystem;

// Configuration structures
struct LocationConfig {
    std::string path;
    std::vector<std::string> methods;
    std::string root;
    std::string index;
    std::string redirect;
    bool autoindex = false;
};

struct ServerConfig {
    int listen_port = 80;
    std::string host = "0.0.0.0";
    std::vector<std::string> server_names;
    std::string root;
    std::string client_max_body_size;
    std::map<int, std::string> error_pages;
    std::vector<LocationConfig> locations;
};

// HTTP Request structure
struct HttpRequest {
    std::string method;
    std::string uri;
    std::string version;
    std::map<std::string, std::string> headers;
};

// HTTP Response structure
struct HttpResponse {
    int status_code;
    std::string status_text;
    std::map<std::string, std::string> headers;
    std::string body;
};

// Utility functions
class HttpUtils {
public:
    static std::string url_decode(const std::string& encoded) {
        std::string decoded;
        for (size_t i = 0; i < encoded.length(); ++i) {
            if (encoded[i] == '%' && i + 2 < encoded.length()) {
                int hex_value;
                std::istringstream hex_stream(encoded.substr(i + 1, 2));
                if (hex_stream >> std::hex >> hex_value) {
                    decoded += static_cast<char>(hex_value);
                    i += 2;
                } else {
                    decoded += encoded[i];
                }
            } else if (encoded[i] == '+') {
                decoded += ' ';
            } else {
                decoded += encoded[i];
            }
        }
        return decoded;
    }

    static std::string get_current_time_rfc1123() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%a, %d %b %Y %H:%M:%S GMT");
        return ss.str();
    }

    static std::string get_mime_type(const std::string& filepath) {
        std::string ext = fs::path(filepath).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        if (ext == ".html" || ext == ".htm") return "text/html";
        if (ext == ".css") return "text/css";
        if (ext == ".js") return "application/javascript";
        if (ext == ".json") return "application/json";
        if (ext == ".xml") return "application/xml";
        if (ext == ".txt" || ext == ".conf" || ext == ".ini") return "text/plain";
        if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
        if (ext == ".png") return "image/png";
        if (ext == ".gif") return "image/gif";
        return "application/octet-stream";
    }
};

class PathValidator {
public:
    static bool validate_uri_path(const std::string& uri_path, std::string& error_msg) {
        // 1. Decode percent-encoded characters
        std::string decoded_path = HttpUtils::url_decode(uri_path);
        
        // 2. Check for directory traversal attempts
        if (!check_traversal_attempts(decoded_path)) {
            error_msg = "Directory traversal attempt detected";
            return false;
        }
        
        // 3. Additional security checks
        if (decoded_path.find('\0') != std::string::npos) {
            error_msg = "Null byte detected in path";
            return false;
        }
        
        return true;
    }

private:
    static bool check_traversal_attempts(const std::string& path) {
        std::vector<std::string> dangerous_patterns = {
            "../", "..\\",
            "%2e%2e%2f", "%2e%2e%5c",
            "%2E%2E%2F", "%2E%2E%5C",
            "..%2f", "..%5c",
            "..%2F", "..%5C"
        };
        
        std::string lower_path = path;
        std::transform(lower_path.begin(), lower_path.end(), lower_path.begin(), ::tolower);
        
        for (const auto& pattern : dangerous_patterns) {
            if (lower_path.find(pattern) != std::string::npos) {
                return false;
            }
        }
        
        return true;
    }
};

class ConfigParser {
public:
    static ServerConfig parse_config(const std::string& config_content) {
        ServerConfig config;
        std::istringstream stream(config_content);
        std::string line;
        bool in_server_block = false;
        bool in_location_block = false;
        LocationConfig current_location;
        
        while (std::getline(stream, line)) {
            // Remove leading/trailing whitespace
            line = trim(line);
            
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') continue;
            
            if (line == "server" || line == "server {") {
                in_server_block = true;
                continue;
            }
            
            if (in_server_block && line == "}") {
                if (in_location_block) {
                    config.locations.push_back(current_location);
                    current_location = LocationConfig{};
                    in_location_block = false;
                } else {
                    in_server_block = false;
                }
                continue;
            }
            
            if (in_server_block) {
                if (line.find("location ") == 0) {
                    if (in_location_block) {
                        config.locations.push_back(current_location);
                        current_location = LocationConfig{};
                    }
                    
                    // Extract location path
                    size_t start = line.find(' ') + 1;
                    size_t end = line.find_last_of(" {");
                    if (end == std::string::npos) end = line.length();
                    current_location.path = line.substr(start, end - start);
                    in_location_block = true;
                    continue;
                }
                
                // Parse server-level directives
                if (!in_location_block) {
                    parse_server_directive(line, config);
                } else {
                    parse_location_directive(line, current_location);
                }
            }
        }
        
        return config;
    }

private:
    static std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }
    
    static void parse_server_directive(const std::string& line, ServerConfig& config) {
        std::istringstream iss(line);
        std::string directive;
        iss >> directive;
        
        if (directive == "listen") {
            iss >> config.listen_port;
        } else if (directive == "host") {
            iss >> config.host;
        } else if (directive == "server_names") {
            std::string name;
            while (iss >> name) {
                if (name.back() == ';') name.pop_back();
                config.server_names.push_back(name);
            }
        } else if (directive == "root") {
            iss >> config.root;
            if (config.root.back() == ';') config.root.pop_back();
        } else if (directive == "client_max_body_size") {
            iss >> config.client_max_body_size;
        } else if (directive == "error_page") {
            std::vector<int> codes;
            std::string token;
            std::string path;
            
            while (iss >> token) {
                if (token.back() == ';') token.pop_back();
                
                try {
                    int code = std::stoi(token);
                    codes.push_back(code);
                } catch (...) {
                    path = token;
                    break;
                }
            }
            
            for (int code : codes) {
                config.error_pages[code] = path;
            }
        }
    }
    
    static void parse_location_directive(const std::string& line, LocationConfig& location) {
        std::istringstream iss(line);
        std::string directive;
        iss >> directive;
        
        if (directive == "methods") {
            std::string method;
            while (iss >> method) {
                if (method.back() == ';') method.pop_back();
                location.methods.push_back(method);
            }
        } else if (directive == "root") {
            iss >> location.root;
            if (location.root.back() == ';') location.root.pop_back();
        } else if (directive == "index") {
            iss >> location.index;
            if (location.index.back() == ';') location.index.pop_back();
        } else if (directive == "redirect") {
            std::string redirect_line;
            std::getline(iss, redirect_line);
            location.redirect = trim(redirect_line);
            if (location.redirect.back() == ';') location.redirect.pop_back();
        } else if (directive == "autoindex") {
            std::string value;
            iss >> value;
            location.autoindex = (value == "on");
        }
    }
};

class HttpServer {
private:
    ServerConfig config;

public:
    HttpServer(const ServerConfig& server_config) : config(server_config) {}
    
    HttpResponse handle_get_request(const HttpRequest& request) {
        std::cout << "Processing GET request for: " << request.uri << std::endl;
        
        // 1. Validate server name
        if (!validate_server_name(request)) {
            return create_error_response(404, "Not Found");
        }
        
        // 2. Validate URI path
        std::string error_msg;
        if (!PathValidator::validate_uri_path(request.uri, error_msg)) {
            std::cout << "Path validation failed: " << error_msg << std::endl;
            return create_error_response(400, "Bad Request");
        }
        
        // 3. Find matching location
        const LocationConfig* location = find_matching_location(request.uri);
        if (!location) {
            std::cout << "No matching location found" << std::endl;
            return create_error_response(404, "Not Found");
        }
        
        std::cout << "Matched location: " << location->path << std::endl;
        
        // 4. Check if method is allowed
        if (!is_method_allowed("GET", *location)) {
            return create_error_response(405, "Method Not Allowed");
        }
        
        // 5. Handle redirect if present
        if (!location->redirect.empty()) {
            return handle_redirect(location->redirect);
        }
        
        // 6. Resolve file path and serve content
        return serve_file(request.uri, *location);
    }

private:
    bool validate_server_name(const HttpRequest& request) {
        if (config.server_names.empty()) return true;
        
        auto host_header = request.headers.find("Host");
        if (host_header == request.headers.end()) return false;
        
        std::string host = host_header->second;
        // Remove port if present
        size_t colon_pos = host.find(':');
        if (colon_pos != std::string::npos) {
            host = host.substr(0, colon_pos);
        }
        
        return std::find(config.server_names.begin(), config.server_names.end(), host) 
               != config.server_names.end();
    }
    
    const LocationConfig* find_matching_location(const std::string& uri) {
        const LocationConfig* best_match = nullptr;
        size_t best_match_length = 0;
        
        for (const auto& location : config.locations) {
            // Check for exact match first
            if (uri == location.path) {
                return &location;
            }
            
            // Check for prefix match
            if (uri.find(location.path) == 0) {
                if (location.path.length() > best_match_length) {
                    best_match = &location;
                    best_match_length = location.path.length();
                }
            }
        }
        
        return best_match;
    }
    
    bool is_method_allowed(const std::string& method, const LocationConfig& location) {
        if (location.methods.empty()) return true;
        
        return std::find(location.methods.begin(), location.methods.end(), method) 
               != location.methods.end();
    }
    
    HttpResponse handle_redirect(const std::string& redirect_config) {
        std::istringstream iss(redirect_config);
        int status_code;
        std::string redirect_url;
        
        iss >> status_code >> redirect_url;
        
        std::cout << "Redirecting with status " << status_code << " to " << redirect_url << std::endl;
        
        HttpResponse response;
        response.status_code = status_code;
        response.status_text = get_status_text(status_code);
        response.headers["Date"] = HttpUtils::get_current_time_rfc1123();
        response.headers["Server"] = "WebServe/1.0";
        response.headers["Location"] = redirect_url;
        response.headers["Content-Length"] = "0";
        response.headers["Connection"] = "keep-alive";
        
        return response;
    }
    
    HttpResponse serve_file(const std::string& uri, const LocationConfig& location) {
        std::string file_path = resolve_file_path(uri, location);
        std::cout << "Attempting to serve file: " << file_path << std::endl;
        
        // Check if file exists
        if (!fs::exists(file_path)) {
            // If it's a directory and autoindex is on, generate directory listing
            if (fs::is_directory(file_path) && location.autoindex) {
                return generate_directory_listing(file_path, uri);
            }
            
            std::cout << "File not found: " << file_path << std::endl;
            return create_error_response(404, "Not Found");
        }
        
        // Check if it's a directory
        if (fs::is_directory(file_path)) {
            if (location.autoindex) {
                return generate_directory_listing(file_path, uri);
            } else {
                return create_error_response(403, "Forbidden");
            }
        }
        
        // Read and serve the file
        return serve_static_file(file_path);
    }
    
    std::string resolve_file_path(const std::string& uri, const LocationConfig& location) {
        std::string root_path = location.root.empty() ? config.root : location.root;
        
        // Remove location prefix from URI
        std::string relative_path = uri;
        if (uri.find(location.path) == 0) {
            relative_path = uri.substr(location.path.length());
        }
        
        // If path is empty or ends with '/', use index file
        if (relative_path.empty() || relative_path == "/" || relative_path.back() == '/') {
            if (!location.index.empty()) {
                return fs::path(root_path) / location.index;
            }
        }
        
        // Remove leading slash from relative path
        if (!relative_path.empty() && relative_path[0] == '/') {
            relative_path = relative_path.substr(1);
        }
        
        return fs::path(root_path) / relative_path;
    }
    
    HttpResponse serve_static_file(const std::string& file_path) {
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            return create_error_response(500, "Internal Server Error");
        }
        
        // Read file content
        std::ostringstream content_stream;
        content_stream << file.rdbuf();
        std::string content = content_stream.str();
        
        // Get file stats
        auto file_time = fs::last_write_time(file_path);
        auto file_size = fs::file_size(file_path);
        
        HttpResponse response;
        response.status_code = 200;
        response.status_text = "OK";
        response.headers["Date"] = HttpUtils::get_current_time_rfc1123();
        response.headers["Server"] = "WebServe/1.0";
        response.headers["Content-Type"] = HttpUtils::get_mime_type(file_path);
        response.headers["Content-Length"] = std::toString(content.length());
        response.headers["Connection"] = "keep-alive";
        response.body = content;
        
        return response;
    }
    
    HttpResponse generate_directory_listing(const std::string& dir_path, const std::string& uri) {
        std::ostringstream html;
        html << "<!DOCTYPE html>\n<html>\n<head>\n";
        html << "<title>Index of " << uri << "</title>\n</head>\n<body>\n";
        html << "<h1>Index of " << uri << "</h1>\n<ul>\n";
        
        // Add parent directory link if not root
        if (uri != "/") {
            html << "<li><a href=\"../\">../</a></li>\n";
        }
        
        try {
            for (const auto& entry : fs::directory_iterator(dir_path)) {
                std::string name = entry.path().filename().string();
                if (entry.is_directory()) {
                    name += "/";
                }
                html << "<li><a href=\"" << name << "\">" << name << "</a></li>\n";
            }
        } catch (const std::exception& e) {
            return create_error_response(500, "Internal Server Error");
        }
        
        html << "</ul>\n</body>\n</html>";
        
        HttpResponse response;
        response.status_code = 200;
        response.status_text = "OK";
        response.headers["Date"] = HttpUtils::get_current_time_rfc1123();
        response.headers["Server"] = "WebServe/1.0";
        response.headers["Content-Type"] = "text/html";
        response.headers["Content-Length"] = std::toString(html.str().length());
        response.headers["Connection"] = "keep-alive";
        response.body = html.str();
        
        return response;
    }
    
    HttpResponse create_error_response(int status_code, const std::string& status_text) {
        HttpResponse response;
        response.status_code = status_code;
        response.status_text = status_text;
        response.headers["Date"] = HttpUtils::get_current_time_rfc1123();
        response.headers["Server"] = "WebServe/1.0";
        response.headers["Connection"] = "keep-alive";
        
        // Check if custom error page exists
        auto error_page_it = config.error_pages.find(status_code);
        if (error_page_it != config.error_pages.end() && fs::exists(error_page_it->second)) {
            std::ifstream file(error_page_it->second);
            if (file.is_open()) {
                std::ostringstream content_stream;
                content_stream << file.rdbuf();
                response.body = content_stream.str();
                response.headers["Content-Type"] = "text/html";
                response.headers["Content-Length"] = std::toString(response.body.length());
                return response;
            }
        }
        
        // Default error page
        std::ostringstream default_error;
        default_error << "<!DOCTYPE html>\n<html>\n<head>\n";
        default_error << "<title>" << status_code << " " << status_text << "</title>\n</head>\n<body>\n";
        default_error << "<h1>" << status_code << " " << status_text << "</h1>\n";
        default_error << "</body>\n</html>";
        
        response.body = default_error.str();
        response.headers["Content-Type"] = "text/html";
        response.headers["Content-Length"] = std::toString(response.body.length());
        
        return response;
    }
    
    std::string get_status_text(int status_code) {
        switch (status_code) {
            case 200: return "OK";
            case 301: return "Moved Permanently";
            case 302: return "Found";
            case 400: return "Bad Request";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 405: return "Method Not Allowed";
            case 500: return "Internal Server Error";
            default: return "Unknown";
        }
    }
};

// Helper function to format HTTP response
std::string format_http_response(const HttpResponse& response) {
    std::ostringstream formatted;
    
    // Status line
    formatted << "HTTP/1.1 " << response.status_code << " " << response.status_text << "\r\n";
    
    // Headers
    for (const auto& header : response.headers) {
        formatted << header.first << ": " << header.second << "\r\n";
    }
    
    // Empty line separating headers from body
    formatted << "\r\n";
    
    // Body
    formatted << response.body;
    
    return formatted.str();
}

// Example usage and testing
int main() {
    // Sample configuration (matching your config file)
    std::string config_content = R"(
server {
    listen 8080;
    host 127.0.0.1;
    server_names example.com www.example.com;
    root /home/asma/webserve/www;
    client_max_body_size 10M;
    
    error_page 404 /home/asma/webserve/www/error/error404.html;
    error_page 500 502 503 504 /home/asma/webserve/www/error/500.html;
    
    location / {
        root /home/asma/webserve/www;
        methods GET POST;
        index index.html index.htm;
        autoindex on;
    }
    
    location /old-page {
        methods GET;
        root /home/asma/webserve/www;
        redirect 301 /new-page;
    }

    location /login {
        methods GET POST;
        index login.html;
        root /home/asma/webserve/www/login;
        redirect 301 /new-page;
    }

    location /signup {
        methods GET POST;
        index login.html;
        root /home/asma/webserve/www/login;
        redirect 301 /new-page;
    }

    location /logout {
        methods POST;
        root /home/asma/webserve/www/login;
        redirect 301 /new-page;
    }
}
)";
    
    // Parse configuration
    ServerConfig config = ConfigParser::parse_config(config_content);
    
    // Create server instance
    HttpServer server(config);
    
    // Test different GET requests
    std::vector<HttpRequest> test_requests = {
        {"GET", "/", "HTTP/1.1", {{"Host", "example.com"}}},
        {"GET", "/login", "HTTP/1.1", {{"Host", "example.com"}}},
        {"GET", "/old-page", "HTTP/1.1", {{"Host", "example.com"}}},
        {"GET", "/nonexistent", "HTTP/1.1", {{"Host", "example.com"}}},
        {"GET", "/../../../etc/passwd", "HTTP/1.1", {{"Host", "example.com"}}},
    };
    
    for (const auto& request : test_requests) {
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "Testing: " << request.method << " " << request.uri << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        
        HttpResponse response = server.handle_get_request(request);
        std::string formatted_response = format_http_response(response);
        
        std::cout << formatted_response << std::endl;
    }
    
    return 0;
}