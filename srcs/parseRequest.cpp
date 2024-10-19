#include "requestHandler.hpp"
#include <unistd.h>
#include <sstream>

std::string	getLine(std::string	str, size_t start) {

	std::string	line;
	size_t		end;

	end = str.find("\r\n", start);
	if (end != std::string::npos)
		line = str.substr(start, end - start);

	return (line);
}

void	requestHandler::parseRequest(void) {

	parseHeaders();
	if (request.extension.compare(".debug") && request.extension.compare(".exit")) {
		if (setPath("www", request._path) > 0)
			request.exists = false;
	}
}

void	requestHandler::parseHeaders(void) {

	std::istringstream	requestStream(request._rawRequest);
	std::string	key;
	std::string	value;

	std::getline(requestStream, request._statusLine);
	parseStatusLine();
	for (std::string line; std::getline(requestStream, line);) {

		if (line.compare("\r")) {
			key = getKey(line);
			value = getValue(line);
			if (request._headers.count(key))
				request._headers[key] = value;
		}
		else {
			for (std::string line; std::getline(requestStream, line);) {
				request._body += line;
				request._body += '\n';
			}
		}
	}
}

void	requestHandler::parseStatusLine(void) {

	size_t start = request._statusLine.find_first_of('/') + 1;
	size_t end = request._statusLine.find_first_of(' ', start);

	request.method = request._statusLine.substr(0, start - 2);
	if (start == end) {
		request._path.append("index.html");
		request.extension = ".html";
	}
	else {
		request._URL = request._statusLine.substr(start, end - start);
		start = request._URL.find_first_of('.');
		if (start == std::string::npos)
			request.extension = ".html";
		else
			request.extension = request._URL.substr(start);
		request._path.append(request._URL);
		if (start == std::string::npos)
			request._path.append(request.extension);
	}
}

std::string	requestHandler::getKey(const std::string &line) {

	std::string	key;
	size_t		end;

	end = line.find_first_of(':');
	if (end == std::string::npos) //! error handling
		exit(1);

	return (line.substr(0, end));
}

std::string	requestHandler::getValue(const std::string &line) {

	std::string value;
	size_t	start;
	size_t	end;

	start = line.find_first_of(':') + 2; //! error handling
	end = line.find_first_of('\r');

	return (line.substr(start, end - start));
}
