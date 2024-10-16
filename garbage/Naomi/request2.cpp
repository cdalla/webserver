#include "request2.hpp"
#include "colours.hpp"
#include <unistd.h>
#include <dirent.h>
// #include <iostream>


Request::Request(Client* ptr) : _ptr(ptr) {

	valRead = 0;
	exists = true;
	initHeaders();
};

Request::~Request(void){};

void	Request::initHeaders(void) {

	_headers["Host"] = "";
	_headers["User-Agent"] = "";
	_headers["Accept"] = "";
	_headers["Accept-Language"] = "";
	_headers["Accept-Encoding"] = "";
	_headers["Referer"] = "";
	_headers["Content-Type"] = "";
	_headers["Content-Length"] = "";
	_headers["Origin"] = "";
	_headers["DNT"] = "";
	_headers["Connection"] = "";
	_headers["Upgrade-Insecure-Request"] = "";
	_headers["Sec-Fetch-Dest"] = "";
	_headers["Sec-Fetch-Mode"] = "";
	_headers["Sec-Fetch-Site"] = "";
	_headers["Sec-Fetch-User"] = "";
	_headers["Pragma"] = "";
	_headers["Cache-Control"] = "";
}



void	Request::readRequest(int server_fd) {
	char	buf[1024];

	valRead = read(connect_fd, buf, 1023);
	if (valRead < 0) {
		perror("reading from connection socket failed");
		close(server_fd);
		close(connect_fd);
		exit(1);
	}
	buf[valRead] = '\0';
	_rawRequest.append(buf);
	parseRequest();
	// printHeaders();
};

int		Request::setPath(std::string current_dir, std::string path) {

	DIR				*dir;
	struct dirent	*entry;
	std::string		tmp;

	dir = opendir(current_dir.c_str());
	if (!dir)
		return (-1);
	entry = readdir(dir);
	while (entry) {
		std::string	dirName(entry->d_name);
		tmp = current_dir;
		// if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && \
		// 	strcmp(entry->d_name, "cgi-logs")) //now also looking in cgi-bin cause want to handle post requests
		if (entry->d_type == DT_DIR && dirName.compare(".") && dirName.compare("..") && dirName.compare("cgi-logs"))
		{
			tmp.append("/");
			tmp.append(entry->d_name);
			if (setPath(tmp, path) == 0) {
				closedir(dir);
				return (0);
			}
		}
		if (entry->d_type == DT_REG && path.compare(entry->d_name) == false) {
			tmp += "/";
			tmp.append(path);
			resource.append(tmp);
			closedir(dir);
			return (0);
		}
		entry = readdir(dir);
	}
	if (current_dir.compare("www") == false)
		resource.append("www/html/404.html");
	closedir(dir);
	return (1);
}

void	Request::printHeaders(void) {

	std::map<std::string, std::string>::iterator it;

	for (it = _headers.begin(); it != _headers.end(); it++) {
		if (it->first.empty())
			std::cout << it->first << ": " << it->second << std::endl;
	}
}

/* Getters */
//! make em return const references, is quicker. Also do more research on references and constness. And getters and setters.

//! Try to understand once more what you are doing by making the getter const, why, and how it affects your program
std::string	Request::getStatusLine(void) const {
	return (_statusLine);
}

std::string	Request::getURL(void) const {
	return (_URL);
}

std::string	Request::getRawRequest(void) const {
	return (_rawRequest);
}

std::string	Request::getBody(void) const {
	return (_body);
}

std::string	Request::getPath(void) const {
	return (_path);
}

std::map<std::string, std::string>	Request::getHeaders(void) const {
	return (_headers);
}

/**	THE INSERTION OPERATOR OVERLOAAAAAD */

void	printSocketInfo(std::ostream& out, Request const &obj);
void	printRequestDetails(std::ostream& out, Request const &obj);

std::ostream&   operator<<(std::ostream& out, Request const &obj) {

	out << CYAN "HTTP Request\n" RST;
	out << "  " U_WHT "General:\n" RST;
	out << "    Method: " << obj.method << "\n";
	out << "    URL: " << obj.getURL() << "\n";
	out << "  " U_WHT "Parsing variables:\n" RST;
	out << "    exists: " << (obj.exists? "true" : "false") << "\n";
	out << "    resource: " << obj.resource << "\n";
	out << "    extension: " << obj.extension << "\n";
	out << "    _path: " << obj.getPath() << "\n";
	// printSocketInfo(out, obj);
	printRequestDetails(out, obj);
	return (out);
}

void	printSocketInfo(std::ostream& out, Request const &obj) {

	out << "  " U_WHT "Socket information:\n" RST;
	out << "    valRead: " << obj.valRead << "\n";
	out << "    connect_fd: " << obj.connect_fd << "\n";
}

void	printRequestDetails(std::ostream& out, Request const &obj) {

	out << "  " U_WHT "Request details:\n" RST;
	out << "    _statusLine: " << obj.getStatusLine() << "\n";
	out << "    _headers:";

	std::map<std::string, std::string>	headers = obj.getHeaders();
	std::map<std::string, std::string>::iterator it;

	for (it = headers.begin(); it != headers.end(); it++) {
		out << (it == headers.begin() ? " " : "              ") << it->first << ": " << (it->second.empty() ? "<empty>" : it->second) << "\n";
	}
	out << "\n    _body: " << (obj.getBody().empty() ? "<empty>" : obj.getBody()) << "\n";
}
