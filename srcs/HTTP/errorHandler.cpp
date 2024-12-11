#include "responseHandler.hpp"
#include <string>

std::string responseHandler::_getStatusMessage(int error) {
    switch (error) {
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 202:
            return "Accepted"; 
        case 204:
            return "No Content"; 
        case 301:
            return "Moved Permanently";
        case 302:    
            return "Found";
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

void responseHandler::_handleErrorPage(int error, std::string error_page) {
    std::string path = _root.empty() ? DEFAULT_ROOT : _root;
    if (path.back() != '/' && error_page.front() != '/') {
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
    if (_client->file_content.empty()) {
        File *file = new File(error_path, _main, _client);
        _client->status = "FILE";
    } else {
        _client->status = "OK";
        _determineType(path);
        _response = "HTTP/1.1 " + std::to_string(error) + " " + _getStatusMessage(error);
        _response += "Content-Type: " + _content_type + "\r\n";
        _response += "Content-Length: " + std::to_string(_client->file_content.length()) + "\r\n";
        _response += "\r\n" + _client->file_content;
        _client->file_content.clear();
    }
}

void responseHandler::_handleDefaultError(int error) {
    std::cout << "Error " << error << std::endl;
    if (error != 204)
        _createErrorPage(error);
    _response = "HTTP/1.1 " + std::to_string(error) + " " + _getStatusMessage(error);
    _response += "Content-Type: text/html\r\n";
    _response += "Content-Length: " + std::to_string(_body.length()) + "\r\n";
    _response += "\r\n" + _body;
}

void responseHandler::_handleError(int error) {
    if (_error_pages.empty()) {
        if (!_config.error_pages.empty()) {
            _error_pages = _config.error_pages;
            _root = !_config.root.empty() ? _config.root : DEFAULT_ROOT;
        } else {
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

void responseHandler::_createErrorPage(int error) {
    std::string errorMessage = std::to_string(error) + " " + _getStatusMessage(error);
    _body = "<!DOCTYPE html>\n<html>\n<head>\n<title>" + errorMessage + "</title>\n</head>\n<body>\n<h1>" + errorMessage + "</h1>\n</body>\n</html>\n";
}