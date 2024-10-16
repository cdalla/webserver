#include "request.hpp"
// #include "utils.hpp"
#include <unistd.h>
#include <sstream>
// #include <iostream>

// void	trimCRLF() {}


std::string	getLine(std::string	str, size_t start) {

	std::string	line;
	size_t		end;

	end = str.find("\r\n", start);
	if (end != std::string::npos)
		line = str.substr(start, end - start);
	return (line);
}

void	Request::parseRequest(void) {
	parseHeaders();
	if (extension.compare(".debug") && extension.compare(".exit"))
	{
		if (setPath("www", _path) > 0)
			exists = false;
	}
}

void	Request::parseHeaders(void) {

	std::istringstream	requestStream(_rawRequest);
	std::string	key;
	std::string	value;

	std::getline(requestStream, _statusLine);
	parseStatusLine();
	for (std::string line; std::getline(requestStream, line);) {

		if (line.compare("\r"))
		{
			key = getKey(line);
			value = getValue(line);
			if (_headers.count(key))
				_headers[key] = value;
		}
		else {
			for (std::string line; std::getline(requestStream, line);) {
				_body += line;
				_body += '\n';
			}
		}
	}
}

void	Request::parseStatusLine(void) {

	size_t start = _statusLine.find_first_of('/') + 1;
	size_t end = _statusLine.find_first_of(' ', start);

	method = _statusLine.substr(0, start - 2);
	if (start == end) {
		_path.append("index.html");
		extension = ".html";
	}
	else {
		_URL = _statusLine.substr(start, end - start);
		start = _URL.find_first_of('.');
		if (start == std::string::npos)
			extension = ".html";
		else
			extension = _URL.substr(start);
		_path.append(_URL);
		if (start == std::string::npos)
			_path.append(extension);
	}
}

std::string	Request::getKey(const std::string &line) {

	std::string	key;
	size_t		end;

	end = line.find_first_of(':');
	if (end == std::string::npos) //! error handling
		exit(1);
	return (line.substr(0, end));
}

std::string	Request::getValue(const std::string &line) {

	std::string value;
	size_t	start;
	size_t	end;

	start = line.find_first_of(':') + 2; //! error handling
	end = line.find_first_of('\r');
	return (line.substr(start, end - start));
}
