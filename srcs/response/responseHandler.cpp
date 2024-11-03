#include "responseHandler.hpp"
#include "colours.hpp"
#include "utils.hpp"
#include <cerrno>
#include <fstream>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <cctype>
#include "CgiHandler.hpp"
#include <dirent.h>
#include <string.h>
#include <algorithm>

#define DEFAULT_ROOT "/var/www/html"

responseHandler::responseHandler(Client *ptr) : _client(ptr), _upload_dir(""), _config(ptr->server->get_config())
{
	_env = new char *[33];
	for (int i = 0; i < 32; ++i)
	{
		_env[i] = NULL;
	}
	_env[32] = NULL;
	_createResponse();
};

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
	if (extension == "html" || extension == "css")
		_content_type = "text/" + extension;
	else if (extension == "jpeg" || extension == "jpg")
		_content_type = "image/jpeg";
	else if (extension == "png")
		_content_type = "image/png";
	else if (extension == "gif")
		_content_type = "image/x-icon";
	else
		_content_type = "text/plain";
}

void responseHandler::_handleError(int error)
{
	_createErrorPage(error);
	_response = std::to_string(error);
	_response.append(" ");
	_response.append(_getStatusMessage(error));
	_response.append("\r\n");
	_response.append("Content-Type: text/html\r\n");
	_response.append("Content-Length: ");
	_response.append(std::to_string(_body.length()));
	_response.append("\r\n\r\n");
	_response.append(_body);
}

void responseHandler::_createErrorPage(int error)
{
	std::string errorPage;
	std::string errorMessage;

	errorMessage = "Error " + std::to_string(error) + " - " + _getStatusMessage(error);
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
	case 505:
		return "HTTP Version Not Supported";
	default:
		return "Internal Server Error";
	}
}

void responseHandler::_handlePage(std::string path)
{

	if (_client->request.method == "POST" || _client->request.method == "DELETE")
	{
		_handleError(405);
		return;
	}

	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		_handleError(500); // Internal Server Error
		return;
	}

	int file_fd = open(path.c_str(), O_RDONLY);
	if (file_fd == -1)
	{
		if (errno == EACCES)
			_handleError(403); // Permission Denied
		else
			_handleError(404); // File Not Found
		close(epoll_fd);
		return;
	}

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = file_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, file_fd, &event) == -1)
	{
		_handleError(500); // Internal Server Error
		close(file_fd);
		close(epoll_fd);
		return;
	}

	struct epoll_event events[1];
	int nfds = epoll_wait(epoll_fd, events, 1, -1);
	if (nfds == -1)
	{
		_handleError(500); // Internal Server Error
		close(file_fd);
		close(epoll_fd);
		return;
	}

	std::ifstream file(path);
	std::string line;

	while (std::getline(file, line))
	{
		_body.append(line);
		_body.append("\n");
	}
	file.close();
	close(file_fd);
	close(epoll_fd);
	_determineType(path);
	_response = "200 OK\r\n";
	_response.append("Content-Type: ");
	_response.append(_content_type);
	_response.append("\r\n");
	_response.append("Content-Length: ");
	_response.append(std::to_string(_body.length()));
	_response.append("\r\n\r\n");
	_response.append(_body);
}

void responseHandler::_handleDirectory(std::string path)
{

	DIR *dir = opendir(path.c_str());
	if (dir == NULL)
		_handleError(404); // File Not Found
	else
	{
		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL)
		{
			if (entry->d_name[0] == '.')
				continue;
			_body.append(entry->d_name);
			_body.append("\n");
		}
		closedir(dir);
		_response = "200 OK\r\n";
		_response.append("Content-Type: text/plain\r\n");
		_response.append("Content-Length: ");
		_response.append(std::to_string(_body.length()));
		_response.append("\r\n\r\n");
		_response.append(_body);
	}
}

void responseHandler::_handleCGI(std::string path)
{
	_createEnv();
	CgiHandler cgi(path.c_str(), _env, _client->request.body.c_str());
	if (cgi.get_status_code() != 200)
	{
		_handleError(cgi.get_status_code());
		return;
	}
	_response = cgi.get_response();
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
	if (_client->request.error)
	{
		_handleError(_client->request.error);
		return;
	}
	_locationHandler(_client->request.uri);
}

void responseHandler::_handleDirRequest(std::string path)
{
	if (_index.empty())
	{
		for (std::vector<std::string>::iterator it = _index.begin(); it != _index.end(); ++it)
		{
			std::string filePath = _root + *it;
			if (access(filePath.c_str(), F_OK) != -1)
			{
				if (filePath.find_last_of(".") != std::string::npos)
				{
					std::string extention = filePath.substr(filePath.find_last_of(".") + 1);
					if (std::find(_cgi_ext.begin(), _cgi_ext.end(), extention) != _cgi_ext.end())
					{
						_handleCGI(filePath);
						return;
					}
					_handlePage(filePath);
					return;
				}
			}
		}
		if (!_autoindex)
		{
			_handleError(403);
			return;
		}
	}
	if (_autoindex)
	{
		_handleDirectory(_root + path);
		return;
	}
	_handleError(404);
}

void responseHandler::_locationHandler(std::string path)
{
	std::string to_locate = path;
	size_t pos;
	Location *matched_location = nullptr;
	for (std::vector<Location>::iterator it = _config.locations.begin(); it != _config.locations.end(); ++it)
	{
		if (path.find(it->path) == 0)
		{ // Path starts with location path
			if (!matched_location || it->path.length() > matched_location->path.length())
			{
				matched_location = &(*it);
			}
		}
	}
	_cgi_ext = _config.cgi_ext;
	_root = !_config.root.empty() ? _config.root : DEFAULT_ROOT;
	_upload_dir = !_config.upload_dir.empty() ? _config.upload_dir : "";
	_autoindex = _config.autoindex;
	_index = _config.index;
	if (!matched_location)
	{
		if (!_config.methods.empty() && 
            std::find(_config.methods.begin(), _config.methods.end(), _client->request.method) == _config.methods.end())
		{
			_handleError(405);
			return;
		}
		if (path.find_last_of(".") != std::string::npos)
		{
			_handlePage(_root + path);
			return;
		}
		_handleDirRequest(_root + path);
		return;
	}
	if (!matched_location->root.empty())
		_root = matched_location->root;
	if (!matched_location->upload_dir.empty())
		_upload_dir = matched_location->upload_dir;
	if (!matched_location->cgi_ext.empty())
		  _cgi_ext.assign(matched_location->cgi_ext.begin(), matched_location->cgi_ext.end());
	if (!matched_location->index.empty())
		 _index.assign(matched_location->index.begin(), matched_location->index.end());
	_autoindex = matched_location->autoindex;

	// Check method permissions
	if (matched_location->methods.empty())
	{
		if (!_config.methods.empty() && 
            std::find(_config.methods.begin(), _config.methods.end(), _client->request.method) == _config.methods.end())
		{
			_handleError(405);
			return;
		}
	}
	else if (std::find(_config.methods.begin(), _config.methods.end(), _client->request.method) == _config.methods.end())
	{
		_handleError(405);
		return;
	}
	// Handle the request
	if (path == to_locate)
	{
		_handleDirRequest(path);
		return;
	}
    std::string request_path = _root + path.substr(to_locate.length());
    std::string extension;
    size_t ext_pos = request_path.find_last_of(".");
    if (ext_pos != std::string::npos)
    {
        extension = request_path.substr(ext_pos + 1);
        if (std::find(_cgi_ext.begin(), _cgi_ext.end(), extension) != _cgi_ext.end())
        {
            _handleCGI(request_path);
            return;
        }
    }
}