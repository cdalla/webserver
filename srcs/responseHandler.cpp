#include "responseHandler.hpp"
#include "colours.hpp"
#include "utils.hpp"
#include <cerrno>
#include <fstream>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <cctype>
#include <dirent.h>
#include <string.h>
#include <algorithm>
#include "file.hpp"

#define DEFAULT_ROOT "/var/www/html"

responseHandler::responseHandler(Client *ptr) : 
    _client(ptr),
    _config(ptr->server->get_config()),
    _main(ptr->main),
    _upload_dir(""),
    _file(""),
    _content_type("text/plain"),
    _body(""),
    _root(""),
    _response(""),
    _env(nullptr),
    _autoindex(false)
{
    try {
        // Allocate environment variables array
        _env = new char*[33];
        for (int i = 0; i < 33; i++) {
            _env[i] = nullptr;
        }

        _createResponse();
    }
    catch (const std::exception& e) {
        // Clean up if constructor throws
        if (_env) {
            for (int i = 0; i < 33; i++) {
                delete[] _env[i];
            }
            delete[] _env;
        }
        throw;
    }
}


responseHandler::~responseHandler(void)
{
	if (_env != NULL)
	{
		for (int i = 0; i < 33; ++i)
		{
			if (_env[i] != NULL)
			{
				delete[] _env[i];
			}
		}
		delete[] _env;
	}
};

std::string responseHandler::get(void)
{
	return _response;
}

void responseHandler::_determineType(std::string path)
{
	std::string extension;
	size_t pos;

	pos = path.find_last_of(".");
	if (pos != std::string::npos)
		extension = path.substr(pos + 1);
	else
		extension = "";
	 if (extension == "html" || extension == "htm") {
        _content_type = "text/html";
    } else if (extension == "css") {
        _content_type = "text/css";
    } else if (extension == "js") {
        _content_type = "application/javascript";
    } else if (extension == "jpg" || extension == "jpeg") {
        _content_type = "image/jpeg";
    } else if (extension == "png") {
        _content_type = "image/png";
    } else if (extension == "gif") {
        _content_type = "image/gif";
    } else if (extension == "txt") {
        _content_type = "text/plain";
    } else {
        _content_type = "application/octet-stream";
    }
}

void responseHandler::_handleErrorPage(int error, std::string error_page)
{
    std::cout << "Reading error page for: " << error << std::endl;
    std::string path = _root;
    if (_root.empty())
        path = _config.root;
    if (path[path.length() - 1] != '/')
        path += "/";
    std::string error_path = path + error_page;
        // Check file permissions and existence
    if (access(error_path.c_str(), F_OK) == -1) {
        std::cout << "File not found: " << path << std::endl;
        _handleDefaultError(404);
    }
    if (access(error_path.c_str(), R_OK) == -1) {
        std::cout << "Permission denied: " << error_path << std::endl;
        _handleDefaultError(403);
        return;
    }
	if (this->_client->file_content.empty()){
		int file_fd = open(error_path.c_str(), O_RDONLY);
		if (file_fd == -1)
		{
			std::cout << "Failed to open file: " << strerror(errno) << std::endl;
			if (errno == EACCES)
				_handleDefaultError(403);
			else
				_handleDefaultError(404);
			return;
		}

		try {
			// Create File handler which will read the entire file
			//File file(file_fd, _main, _client);
			
			// Determine content type and create response
			_determineType(path);
				_response = "HTTP/1.1 ";
				_response += std::to_string(error);
				_response += " ";
				_response += _getStatusMessage(error);
				_response += "Content-Type: " + _content_type + "\r\n";
				_response += "Content-Length: " + std::to_string(_client->file_content.length()) + "\r\n";
				_response += "\r\n" + _client->file_content;
			
				std::cout << "Successfully read file and created response" << std::endl;
				std::cout << "Response: " << _response << std::endl;
		}
		catch (const std::exception& e) {
			std::cerr << "Error handling file: " << e.what() << std::endl;
			_handleDefaultError(500);
		}
	}

}
void responseHandler::_handleDefaultError(int error)
{
    std::cout << "Creating default error page for: " << error << std::endl;
    _createErrorPage(error);
    _response = "HTTP/1.1 ";
    _response += std::to_string(error);
    _response += " ";
    _response += _getStatusMessage(error);
    _response += "Content-Type: text/html\r\n";
    _response += "Content-Length: " + std::to_string(_body.length()) + "\r\n";
    _response += "\r\n" + _body;
}

void responseHandler::_handleError(int error)
{   
    
    std::string error_page = _error_pages[error];
    if (error_page.empty()) {
        std::cout << "No error page found for: " << error << std::endl;
        _handleDefaultError(error);
        return;
    }
    _handleErrorPage(error, error_page);
}

void responseHandler::_createErrorPage(int error)
{
	std::string errorPage;
	std::string errorMessage;

	errorMessage = std::to_string(error) + " " + _getStatusMessage(error);
	errorPage = "<!DOCTYPE html>\n<html>\n<head>\n<title>" + errorMessage + "</title>\n</head>\n<body>\n<h1>" + errorMessage + "</h1>\n</body>\n</html>\n";
	_body = errorPage;
}

std::string responseHandler::_getStatusMessage(int error)
{
	switch (error)
	{
	case 400:
		return "Bad Request";
	case 403:
		return "Forbidden";
	case 404:
		return "Not Found";
	case 405:
		return "Method Not Allowed";
	case 413:
		return "Payload Too Large";
    case 502:
        return "Bad Gateway";
    case 503:
        return "Service Unavailable";
    case 504:
        return "Gateway Timeout";
	case 505:
		return "HTTP Version Not Supported";
	default:
		return "Internal Server Error";
	}
}

void responseHandler::_handlePage(std::string path)
{
        std::cout << "path: " << path << std::endl;
 if (_client->request.method == "POST" || _client->request.method == "DELETE")
    {
        std::cout << "Method not allowed" << std::endl;
        _handleError(405);
        return;
    }

    // Check file permissions and existence
    if (access(path.c_str(), F_OK) == -1) {
        std::cout << "File not found: " << path << std::endl;
        _handleError(404);
        return;
    }
    
    if (access(path.c_str(), R_OK) == -1) {
        std::cout << "Permission denied: " << path << std::endl;
        _handleError(403);
        return;
    }
    //std::cout << "file content: \n" << _client->file_content << std::endl;
	if (_client->file_content.empty())
	{
    // int file_fd = open(path.c_str(), O_RDONLY);
    // if (file_fd == -1)
    // {
    //     std::cout << "Failed to open file: " << strerror(errno) << std::endl;
    //     if (errno == EACCES)
    //         _handleError(403);
    //     else
    //         _handleError(404);
    //     return;
    // }

    try {
        // Create File handler which will read the entire file
		std::cout << "creating new file handler" << std::endl;
        File *file = new File(path, _main, _client);
        _client->status = "FILE";
        //Determine content type and create response
        // _determineType(path);
        // _response = "HTTP/1.1 200 OK\r\n";
        // _response += "Content-Type: " + _content_type + "\r\n";
        // _response += "Content-Length: " + std::to_string(_client->file_content.length()) + "\r\n";
        // _response += "\r\n" + _client->file_content;
        
        // std::cout << "Successfully read file and created response" << std::endl;
        //  std::cout << "Response: " << _response << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error handling file: " << e.what() << std::endl;
        _handleError(500);
    }
	}
    else
    {
        _client->status = "OK";
        _determineType(path);
        _response = "HTTP/1.1 200 OK\r\n";
        _response += "Content-Type: " + _content_type + "\r\n";
        _response += "Content-Length: " + std::to_string(_client->file_content.length()) + "\r\n";
        _response += "\r\n" + _client->file_content;
        
        std::cout << "Successfully read file and created response" << std::endl;
    }
}

void responseHandler::_handleDirectory(std::string path)
{
        std::cout << "path: " << path << std::endl;

	DIR *dir = opendir(path.c_str());
	if (dir == NULL) {
		std::cout << "Error opening directory: " << strerror(errno) << std::endl;
		_handleError(404);
		return;
	}
	
	// Build HTML directory listing
	_body = "<!DOCTYPE html>\n<html>\n<head>\n";
	_body += "<title>Directory listing</title>\n";
	_body += "<style>body{font-family:Arial,sans-serif;margin:20px;}\n";
	_body += "a{text-decoration:none;color:#0066cc;display:block;padding:5px;}\n";
	_body += "a:hover{background:#f0f0f0;}</style>\n";
	_body += "</head>\n<body>\n";
	_body += "<h1>Directory listing for " + _client->request.uri + "</h1>\n";
	_body += "<div>\n";
	
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
	if (entry->d_name[0] == '.' && entry->d_name[1] == '\0')
	    continue;  // Skip current directory
	
	_body += "<a href=\"";
	if (_client->request.uri.back() != '/') {
	    _body += _client->request.uri + "/" + entry->d_name;
	} else {
	    _body += _client->request.uri + entry->d_name;
	}
	_body += "\">" + std::string(entry->d_name) + "</a>\n";
	}
	
	_body += "</div>\n</body>\n</html>";
	closedir(dir);
	
	// Create complete response with headers
	_response = "HTTP/1.1 200 OK\r\n";
	_response += "Content-Type: text/html\r\n";
	_response += "Content-Length: " + std::to_string(_body.length()) + "\r\n";
	_response += "Connection: keep-alive\r\n";
	_response += "\r\n";  // Empty line between headers and body
	_response += _body;  // Add the body
	
	// Set client status to indicate response is complete
	_client->status = "done";
}

void responseHandler::_handleCGI(std::string path)
{
	if (_client->cgi_result.empty())
	{
    std::cout << "Handling CGI request: " << path << std::endl;
	  // Add these checks
    if (access(path.c_str(), F_OK) == -1) {
        std::cerr << "CGI script not found: " << path << std::endl;
        _handleError(404);
        return;
    }
    if (access(path.c_str(), X_OK) == -1) {
        std::cerr << "CGI script not executable: " << path << std::endl;
        _handleError(403);
        return;
    }

    _createEnv();
    try {
        // Create new Cgi instance instead of CgiHandler
        Cgi* cgi = new Cgi(_main, path.c_str(), _env, 
        _client->request.body.empty() ? "" :_client->request.body.c_str(), _client);
        
        // The response will be set in client->cgi_result when CGI processing completes
        // For now, we'll send a preliminary response
        _response = "HTTP/1.1 200 OK\r\n";
        _response += "Content-Type: text/html\r\n";
        _response += "\r\n";
    }
    catch (const WebservException& e) {
        std::cerr << "CGI initialization failed: " << e.what() << std::endl;
        _handleError(500);
    }
	}
	else
		_response = _client->cgi_result;
}

static char *joing_string(const char *str1, const char *str2)
{
	size_t size = strlen(str1) + strlen(str2);
	char *string = new char[size + 1];
	strcpy(string, str1);
	strcat(string, str2);
	string[size] = '\0';
	return (string);
}

void responseHandler::_createEnv(void)
{
	_env[0] = joing_string("UPLOAD_DIR=", _upload_dir.c_str());
	_env[1] = joing_string("DOCUMENT_ROOT=", _root.c_str());
	_env[2] = joing_string("SCRIPT_FILENAME=", (_root + _client->request.script_name).c_str());
	_env[3] = joing_string("PATH_INFO=", _client->request.uri.c_str());
	_env[4] = joing_string("PATH_TRANSLATED=", _client->request.uri.c_str());
	_env[5] = joing_string("QUERY_STRING=", _client->request.query_string.c_str());
	_env[6] = joing_string("REMOTE_ADDR=", "");
	_env[7] = joing_string("REMOTE_IDENT=", "");
	_env[8] = joing_string("REMOTE_USER=", "");
	_env[9] = joing_string("REQUEST_METHOD=", _client->request.method.c_str());
	_env[10] = joing_string("REQUEST_URI=", _client->request.uri.c_str());
	_env[11] = joing_string("SCRIPT_NAME=", _client->request.script_name.c_str());
	_env[12] = joing_string("SERVER_NAME=", _config.server_name.c_str());
	_env[13] = joing_string("SERVER_PORT=", std::to_string(_config.listen).c_str());
	_env[14] = joing_string("SERVER_PROTOCOL=", "HTTP/1.1");
	_env[15] = joing_string("SERVER_SOFTWARE=", "webserv/1.0");
	_env[16] = joing_string("CONTENT_LENGTH=", std::to_string(_client->request.body.length()).c_str());
	_env[17] = joing_string("CONTENT_TYPE=", _client->request.headers["Content-Type"].c_str());
	_env[18] = joing_string("HTTP_ACCEPT=", _client->request.headers["Accept"].c_str());
	_env[19] = joing_string("HTTP_ACCEPT_CHARSET=", _client->request.headers["Accept-Charset"].c_str());
	_env[20] = joing_string("HTTP_ACCEPT_ENCODING=", _client->request.headers["Accept-Encoding"].c_str());
	_env[21] = joing_string("HTTP_ACCEPT_LANGUAGE=", _client->request.headers["Accept-Language"].c_str());
	_env[22] = joing_string("HTTP_CONNECTION=", _client->request.headers["Connection"].c_str());
	_env[23] = joing_string("HTTP_HOST=", _client->request.headers["Host"].c_str());
	_env[24] = joing_string("HTTP_REFERER=", _client->request.headers["Referer"].c_str());
	_env[25] = joing_string("HTTP_USER_AGENT=", _client->request.headers["User-Agent"].c_str());
	_env[26] = joing_string("REDIRECT_STATUS=", "200");
	_env[27] = joing_string("REDIRECT_URL=", "");
	_env[28] = joing_string("REDIRECT_URI=", "");
	_env[29] = joing_string("GATEWAY_INTERFACE=", "CGI/1.1");
	_env[30] = joing_string("SERVER_ADMIN=", "");
	_env[31] = joing_string("SERVER_SIGNATURE=", "");
}

void responseHandler::_createResponse(void)
{
	 try {
        if (_client->request.error) {
            _handleError(_client->request.error);
            return;
        }

        // Start with HTTP/1.1
        _response = "HTTP/1.1 ";
        
        // Add basic headers if they're missing
        if (_client->request.headers.find("Host") == _client->request.headers.end()) {
            _handleError(400);
            return;
        }

        // Process the request
        _locationHandler(_client->request.uri);
        
        // Make sure we have some kind of response
        if (_response.empty()) {
            _handleError(500);
            return;
        }

        std::cout << "Created response of size: " << _response.size() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error creating response: " << e.what() << std::endl;
        _handleError(500);
    }
}


void responseHandler::_handleDirRequest(std::string path)
{
	 if (_index.empty()) {
        if (!_autoindex) {
            std::cout << "No index files found and autoindex disabled" << std::endl;
            _handleError(403);
            return;
        }
        std::cout << "No index files found, using autoindex" << std::endl;
        _handleDirectory(path);
        return;
    }

    // Try each index file
    for (std::vector<std::string>::iterator it = _index.begin(); 
         it != _index.end(); ++it) 
    {
        std::cout << "Checking for index file: " << *it << std::endl;
        if (path.empty() || path[path.length() - 1] != '/') {
            path += "/";
        }
        std::string full_path = path + *it;
        std::cout << "Checking for index file: " << full_path << std::endl;
        if (access(full_path.c_str(), F_OK) != -1) {
            std::cout << "Found index file: " << full_path << std::endl;
            // Check if it's a CGI file
            size_t ext_pos = it->find_last_of(".");
            if (ext_pos != std::string::npos) {
                std::string ext = it->substr(ext_pos);
                if (std::find(_cgi_ext.begin(), _cgi_ext.end(), ext) != _cgi_ext.end()) {
                    std::cout << "Found CGI index file" << std::endl;
                    _handleCGI(full_path);
                    return;
                }
            }
            std::cout << "Serving index file" << std::endl;
            _handlePage(full_path);
            return;
        }
    }

    // If autoindex is enabled and no index file found
    if (_autoindex) {
        _handleDirectory(_root + path);
        return;
    }

    _handleError(404);
}

void responseHandler::_locationHandler(std::string path)
{

     // Reset to server defaults
    _cgi_ext = _config.cgi_ext;
    _root = !_config.root.empty() ? _config.root : "/var/www/html";
    _upload_dir = _config.upload_dir;
    _autoindex = _config.autoindex;
    _index = _config.index;
    _error_pages = _config.error_pages;

    // Find best matching location using a pointer to avoid copying
    const Location* matched_location = NULL;
    size_t longest_match = 0;
    std::string location_prefix;

      // 1. First look for exact full path match
    for (std::vector<Location>::const_iterator it = _config.locations.begin(); 
         it != _config.locations.end(); ++it) {
        if (path == it->path) {
            matched_location = &(*it);
            location_prefix = it->path;
            std::cout << "Found exact path match: " << it->path << std::endl;
            break;
        }
    }

    // 2. If no exact match, check for extension match
    if (!matched_location) {
        size_t ext_pos = path.find_last_of(".");
        if (ext_pos != std::string::npos) {
            std::string extension = path.substr(ext_pos);
            for (std::vector<Location>::const_iterator it = _config.locations.begin(); 
                 it != _config.locations.end(); ++it) {
                if (it->path == extension) {
                    matched_location = &(*it);
                    location_prefix = path.substr(0, ext_pos + extension.length());
                    std::cout << "Found extension match: " << it->path << std::endl;
                    break;
                }
            }
        }
    }

    // 3. If still no match, try prefix matching (longest wins)
    if (!matched_location) {
        for (std::vector<Location>::const_iterator it = _config.locations.begin(); 
             it != _config.locations.end(); ++it) {
            if (path.compare(0, it->path.length(), it->path) == 0) {
                if (!matched_location || it->path.length() > longest_match) {
                    matched_location = &(*it);
                    location_prefix = it->path;
                    longest_match = it->path.length();
                    std::cout << "Found prefix match: " << it->path << std::endl;
                }
            }
        }
    }
    if (!matched_location) {
		std::cout << "No location found" << std::endl;
        if (!_config.methods.empty() && 
            std::find(_config.methods.begin(), _config.methods.end(), 
                     _client->request.method) == _config.methods.end()) {
			std::cout << "Method not allowed" << std::endl;
            _handleError(405);
            return;
        }
    } else {
        std::cout << "Location found: " << matched_location->path << std::endl;    
		std::list<std::string>::const_iterator method_it;
        bool method_found = false;
        
        if (!matched_location->methods.empty()) {
            for (method_it = matched_location->methods.begin(); 
                 method_it != matched_location->methods.end(); ++method_it) {
                if (*method_it == _client->request.method) {
                    method_found = true;
                    break;
                }
            }
            if (!method_found) {
				std::cout << "Method not allowed" << std::endl;
                _handleError(405);
                return;
            }
        }

        // Update configuration with location specific settings
        if (!matched_location->root.empty())
            _root = matched_location->root;
        if (!matched_location->upload_dir.empty())
            _upload_dir = matched_location->upload_dir;
        if (!matched_location->cgi_ext.empty())
            _cgi_ext.assign(matched_location->cgi_ext.begin(), 
                          matched_location->cgi_ext.end());
        if (!matched_location->index.empty()){
            _index.assign(matched_location->index.begin(), 
                        matched_location->index.end());
        }
        if (!matched_location->error_pages.empty())
            _error_pages = matched_location->error_pages;
        _autoindex = matched_location->autoindex;
    }

     // Remove the location prefix from the path for proper file handling
    std::string adjusted_path = path;
    if (matched_location && path.find(location_prefix) == 0) {
        adjusted_path = path.substr(location_prefix.length());
        if (_root[_root.length() - 1] != '/') {
            _root += "/";
        }
    }

    // Process the request
    std::string full_path = _root + adjusted_path;
    std::cout << "Full path: " << full_path << std::endl;

    // Check if it's a CGI request based on extension
    size_t ext_pos = path.find_last_of(".");
    if (ext_pos != std::string::npos) {
        std::string extension = path.substr(ext_pos);
        std::vector<std::string>::const_iterator cgi_it;
        for (cgi_it = _cgi_ext.begin(); cgi_it != _cgi_ext.end(); ++cgi_it) {
            if (*cgi_it == extension) {
                std::cout << "CGI request" << std::endl;
                std::string script_name = path.substr(path.find_last_of("/") + 1);
                std::string cgi_path = _root + "/" + script_name;
                _handleCGI(cgi_path);
                return;
            }
        }
    }

    // Check if path is a directory using opendir
    DIR* dir = opendir(full_path.c_str());
    if (dir != NULL) {
        std::cout << "Directory request" << std::endl;
        closedir(dir);
        _handleDirRequest(full_path);
        return;
    }
    std::cout << "File request" << std::endl;
    // Handle as regular file
_handlePage(full_path);
}
