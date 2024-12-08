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
    _autoindex(false),
    _index(),
    _cgi_ext()
{
    _env = new char*[33];
    for (int i = 0; i < 33; i++) {
        _env[i] = nullptr;
    }
    _createResponse();
}

responseHandler::~responseHandler(void){
	if (_env != NULL){
		for (int i = 0; i < 33; ++i){
			if (_env[i] != NULL){
				delete[] _env[i];
			}
		}
		delete[] _env;
	}
};

std::string responseHandler::get(void){
	return _response;
}

void responseHandler::_determineType(std::string path){
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

void responseHandler::_handleErrorPage(int error, std::string error_page){
    std::string path = _root;
    if (_root.empty()) {
        path = DEFAULT_ROOT;
    }
    if (path[path.length() - 1] != '/' && error_page[0] != '/'){
        path += "/";
    }
    std::string error_path = path + error_page;
    if (access(error_path.c_str(), F_OK) == -1) {
        _handleDefaultError(404);
    }
    if (access(error_path.c_str(), R_OK) == -1) {
        _handleDefaultError(403);
        return;
    }
	if (this->_client->file_content.empty()){
		File *file = new File(error_path, _main, _client);
        _client->status = "FILE";
	} else {
        _client->status = "OK";
		_determineType(path);
        _response = "HTTP/1.1 ";
        _response += std::to_string(error);
        _response += " ";
        _response += _getStatusMessage(error);
        _response += "Content-Type: " + _content_type + "\r\n";
        _response += "Content-Length: " + std::to_string(_client->file_content.length()) + "\r\n";
        _response += "\r\n" + _client->file_content;
        _client->file_content.clear();
	}
}


void responseHandler::_handleDefaultError(int error){
    _createErrorPage(error);
    _response = "HTTP/1.1 ";
    _response += std::to_string(error);
    _response += " ";
    _response += _getStatusMessage(error);
    _response += "Content-Type: text/html\r\n";
    _response += "Content-Length: " + std::to_string(_body.length()) + "\r\n";
    _response += "\r\n" + _body;
}

void responseHandler::_handleError(int error){ 
    if (_error_pages.empty()) {
        if (!_config.error_pages.empty()) {
            _error_pages = _config.error_pages;
            _root =  !_config.root.empty() ? _config.root : DEFAULT_ROOT;
        }
        else {
            _handleDefaultError(error);
            return;
        }
    }
    std::string error_page = _error_pages[error];
    if (error_page.empty()) {
        _handleDefaultError(error);
        return;
    }
    _handleErrorPage(error, error_page);
}

void responseHandler::_createErrorPage(int error){
	std::string errorPage;
	std::string errorMessage;

	errorMessage = std::to_string(error) + " " + _getStatusMessage(error);
	errorPage = "<!DOCTYPE html>\n<html>\n<head>\n<title>" + errorMessage + "</title>\n</head>\n<body>\n<h1>" + errorMessage + "</h1>\n</body>\n</html>\n";
	_body = errorPage;
}

std::string responseHandler::_getStatusMessage(int error){
	switch (error){
        case 400:
            return "Bad Request";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
		case 408:
			return "Request Timeout";
        case 413:
            return "Payload Too Large";
		case 501:
			return "Not Implemented";
        case 502:
            return "Bad Gateway";
        case 503:
            return "Service Unavailable";
        case 504:
            return "Gateway Timeout";
        case 505:
            return "HTTP Version Not Supported";
		case 4444:
			return "Carlo the King";
        default:
            return "Internal Server Error";
	}
}

void responseHandler::_handlePage(std::string path){
    //std::cout << "path: " << path << std::endl;
    if (_client->request.method == "POST" || _client->request.method == "DELETE"){
       //std::cout << "Method not allowed" << std::endl;
        _handleError(405);
        return;
    }

    // Check file permissions and existence
    if (access(path.c_str(), F_OK) == -1) {
        _handleError(404);
        return;
    }
    
    if (access(path.c_str(), R_OK) == -1) {
        _handleError(403);
        return;
    }

	if (_client->file_content.empty()){
        File *file = new File(path, _main, _client);
        _client->status = "FILE";
	} else {
        _client->status = "OK";
        _determineType(path);
        _response = "HTTP/1.1 200 OK\r\n";
        _response += "Content-Type: " + _content_type + "\r\n";
        _response += "Content-Length: " + std::to_string(_client->file_content.length()) + "\r\n";
        _response += "\r\n" + _client->file_content;
        _client->file_content.clear();
    }
}

void responseHandler::_handleDirectory(std::string path){
	DIR *dir = opendir(path.c_str());
	if (dir == NULL) {
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

	_response = "HTTP/1.1 200 OK\r\n";
	_response += "Content-Type: text/html\r\n";
	_response += "Content-Length: " + std::to_string(_body.length()) + "\r\n";
	_response += "Connection: keep-alive\r\n";
	_response += "\r\n";
	_response += _body;
	_client->status = "done";
}

void responseHandler::_handleCGI(std::string path){
	//std::cout << "body size: " << _client->request.body.size() << std::endl;
	if (_client->cgi_result.empty()){
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
        if (_client->file_content.empty()){
            // Create new Cgi instance instead of CgiHandler
            //std::cout << "BODY: \n" << _client->request.body << std::endl;
            //std::cout << "BODY sixe: \n" << _client->request.body.size() << std::endl;
            Cgi* cgi = new Cgi(_main, path.c_str(), _env, _client->request.body.empty() ? "" :_client->request.body, _client);
            _client->status = "CGI";
        } else {
            _response += _client->file_content;
            _client->file_content.clear();
        }
    }
}

static char *joing_string(const char *str1, const char *str2){
	size_t size = strlen(str1) + strlen(str2);
	char *string = new char[size + 1];
	strcpy(string, str1);
	strcat(string, str2);
	string[size] = '\0';
	return (string);
}

void responseHandler::_createEnv(void){
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

void responseHandler::_createResponse(void){
    if (_client->request.error) {
        _handleError(_client->request.error);
        return;
    }
    _response = "HTTP/1.1 ";
    _locationHandler(_client->request.uri);
    if (_response.empty()) {
        _handleError(500);
        return;
    }
}

void responseHandler::_handleDirRequest(std::string path)
{
	 if (_index.empty()) {
        if (!_autoindex) {
            _handleError(403);
            return;
        }
        _handleDirectory(path);
        return;
    }
    // Try each index file
    for (std::vector<std::string>::iterator it = _index.begin(); it != _index.end(); ++it) {
        if (path.empty() || path[path.length() - 1] != '/') {
            path += "/";
        }
        std::string full_path = path + *it;
        if (access(full_path.c_str(), F_OK) != -1) {
            size_t ext_pos = it->find_last_of(".");
            if (ext_pos != std::string::npos) {
                std::string ext = it->substr(ext_pos);
                if (std::find(_cgi_ext.begin(), _cgi_ext.end(), ext) != _cgi_ext.end()) {
                    _handleCGI(full_path);
                    return;
                }
            }
            _handlePage(full_path);
            return;
        }
    }
    if (_autoindex) {
        _handleDirectory(_root + path);
        return;
    }
    _handleError(404);
}

void responseHandler::_handleRedirect(std::string path) {
    _response = "HTTP/1.1 302 Found\r\n";
    if (path.find("http://") == 0 || path.find("https://") == 0) {
        _response += "Location: " + path + "\r\n";
    } else {
        _response += "Location: " + path + "/\r\n";
    }
    _response += "Content-Length: 0\r\n\r\n";
}

void responseHandler::_locationHandler(std::string path){
    // Reset to server defaults
    _cgi_ext = _config.cgi_ext;
    _root = !_config.root.empty() ? _config.root : DEFAULT_ROOT;
    _upload_dir = _config.upload_dir;
    _autoindex = _config.autoindex;
    _index = _config.index;
    _error_pages = _config.error_pages;
	_redirect_url = _config.redirect_url;

    // Find best matching location using a pointer to avoid copying
    const Location* matched_location = NULL;
    size_t longest_match = 0;
    std::string location_prefix;

    // 1. First look for exact full path match
    for (std::vector<Location>::const_iterator it = _config.locations.begin(); it != _config.locations.end(); ++it) {
        if (path == it->path) {
            matched_location = &(*it);
            location_prefix = it->path;
            break;
        }
    }

    // 2. If no exact match, check for extension match
    if (!matched_location) {
        size_t ext_pos = path.find_last_of(".");
        if (ext_pos != std::string::npos) {
            std::string extension = path.substr(ext_pos);
            for (std::vector<Location>::const_iterator it = _config.locations.begin(); it != _config.locations.end(); ++it) {
                if (it->path == extension) {
                    matched_location = &(*it);
                    location_prefix = path.substr(0, ext_pos + extension.length());
                    break;
                }
            }
        }
    }

    // 3. If still no match, try prefix matching (longest wins)
    if (!matched_location) {
        for (std::vector<Location>::const_iterator it = _config.locations.begin(); it != _config.locations.end(); ++it) {
            if (path.compare(0, it->path.length(), it->path) == 0) {
                if (!matched_location || it->path.length() > longest_match) {
                    matched_location = &(*it);
                    location_prefix = it->path;
                    longest_match = it->path.length();
                }
            }
        }
    }
    if (!matched_location) {
        if (!_config.methods.empty() && 
            std::find(_config.methods.begin(), _config.methods.end(), 
                     _client->request.method) == _config.methods.end()) {
            _handleError(405);
            return;
        }
    }  else {    
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
        _error_pages = matched_location->error_pages;
        _autoindex = matched_location->autoindex;
        _redirect_url = matched_location->redirect_url;
    }
      
     // Remove the location prefix from the path for proper file handling
    std::string adjusted_path = path;
    if (matched_location && path.find(location_prefix) == 0) {
        adjusted_path = path.substr(location_prefix.length());
        if (!_root.empty() && _root[_root.length() - 1] != '/') {
            _root += "/";
        }
    }
    if (adjusted_path[0] == '/') {
       adjusted_path.erase(0, 1);
    }
    if (!_redirect_url.empty()) {
        _handleRedirect(_redirect_url + adjusted_path);
        return;
    }
    
    // Process the request
    std::string full_path = _root + adjusted_path;
    size_t ext_pos = path.find_last_of(".");
    if (ext_pos != std::string::npos) {
        std::string extension = path.substr(ext_pos);
        std::vector<std::string>::const_iterator cgi_it;
        for (cgi_it = _cgi_ext.begin(); cgi_it != _cgi_ext.end(); ++cgi_it) {
            if (*cgi_it == extension) {
                std::string script_name = path.substr(path.find_last_of("/") + 1);
                std::string cgi_path = _root + script_name;
                _handleCGI(cgi_path);
                return;
            }
        }
    }

    DIR* dir = opendir(full_path.c_str());
    if (dir != NULL) {
        closedir(dir);
        if (path[path.length() - 1] != '/') {
            _handleRedirect(path);
            return;
        }
        _handleDirRequest(full_path);
        return;
    }
    _handlePage(full_path);
}
