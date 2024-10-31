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

responseHandler::responseHandler(Client *ptr) : _ptr(ptr)
{
	_env = new char *[33];
	for (int i = 0; i < 32; ++i)
	{
		_env[i] = NULL;
	}
	_env[32] = NULL;
};

responseHandler::~responseHandler(void)
{
	if (_env != NULL)
	{
		for (int i = 0; i < 33; ++i)
		{
			if (request.env[i] != NULL)
			{
				delete[] request.env[i];
			}
		}
		delete[] request.env;
	}
	if (request.script_name != NULL)
		delete[] request.script_name;
};

std::string responseHandler::get(void)
{
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
	else if (extension == "js")
		_content_type = "application/javascript";
	else if (extension == "jpeg" || extension == "jpg")
		_content_type = "image/jpeg";
	else if (extension == "png")
		_content_type = "image/png";
	else if (extension == "gif")
		_content_type = "image/gif";
	else if (extension == "svg")
		_content_type = "image/svg+xml";
	else if (extension == "ico")
		_content_type = "image/x-icon";
	else if (extension == "txt")
		_content_type = "text/plain";
	else
		_content_type = "application/octet-stream";
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
	case 501:
		return "Not Implemented";
	case 505:
		return "HTTP Version Not Supported";
	default:
		return "Internal Server Error";
	}
}

void responseHandler::_handlePage(std::string path)
{

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

	void responseHandler::_handleCGI(std::string path)
	{
		_createEnv();
		CgiHandler cgi(path.c_str(), _ptr->_env, _ptr->request.body.c_str());
		if (cgi.get_status_code() != 200)
		{
			_handleError(cgi.get_status_code());
			return;
		}
		_response = cgi.get_response();
	}
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
	_env[0] = joing_string("COMSPEC=", "");
	_env[1] = joing_string("DOCUMENT_ROOT=", _ptr->_config.root.c_str());
	_env[2] = joing_string("SCRIPT_FILENAME=", (_ptr->_config.root + request.script_name).c_str());
	_env[3] = joing_string("PATH_INFO=", request.uri.c_str());
	_env[4] = joing_string("PATH_TRANSLATED=", request.uri.c_str());
	_env[5] = joing_string("QUERY_STRING=", request.query_string.c_str());
	_env[6] = joing_string("REMOTE_ADDR=", _ptr->_client_ip.c_str());
	_env[7] = joing_string("REMOTE_IDENT=", "");
	_env[8] = joing_string("REMOTE_USER=", "");
	_env[9] = joing_string("REQUEST_METHOD=", request.method.c_str());
	_env[10] = joing_string("REQUEST_URI=", request.uri.c_str());
	_env[11] = joing_string("SCRIPT_NAME=", request.script_name);
	_env[12] = joing_string("SERVER_NAME=", _ptr->_config.server_name.c_str());
	_env[13] = joing_string("SERVER_PORT=", std::to_string(_ptr->_config.listen).c_str());
	_env[14] = joing_string("SERVER_PROTOCOL=", "HTTP/1.1");
	_env[15] = joing_string("SERVER_SOFTWARE=", "webserv/1.0");
	_env[16] = joing_string("CONTENT_LENGTH=", std::to_string(request.body.length()).c_str());
	_env[17] = joing_string("CONTENT_TYPE=", request.headers["Content-Type"].c_str());
	_env[18] = joing_string("HTTP_ACCEPT=", request.headers["Accept"].c_str());
	_env[19] = joing_string("HTTP_ACCEPT_CHARSET=", request.headers["Accept-Charset"].c_str());
	_env[20] = joing_string("HTTP_ACCEPT_ENCODING=", request.headers["Accept-Encoding"].c_str());
	_env[21] = joing_string("HTTP_ACCEPT_LANGUAGE=", request.headers["Accept-Language"].c_str());
	_env[22] = joing_string("HTTP_CONNECTION=", request.headers["Connection"].c_str());
	_env[23] = joing_string("HTTP_HOST=", request.headers["Host"].c_str());
	_env[24] = joing_string("HTTP_REFERER=", request.headers["Referer"].c_str());
	_env[25] = joing_string("HTTP_USER_AGENT=", request.headers["User-Agent"].c_str());
	_env[26] = joing_string("REDIRECT_STATUS" =, "200");
	_env[27] = joing_string("REDIRECT_URL=", request.uri.c_str());
	_env[28] = joing_string("REDIRECT_URI=", request.uri.c_str());
	_env[29] = joing_string("GATEWAY_INTERFACE=", "CGI/1.1");
	_env[30] = joing_string("SERVER_ADMIN=", "");
	_env[31] = joing_string("SERVER_SIGNATURE=", "");
}

void responseHandler::_createResponse(void)
{
	if (request.error)
	{
		_handleError(request.error);
		return;
	}
	_locationHandler();
}

void responseHandler::_locationHandler(std::string path)
{
	
	
}