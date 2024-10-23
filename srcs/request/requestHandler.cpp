#include "requestHandler.hpp"
# include "client.hpp"

#include "colours.hpp"
#include <unistd.h>
#include <dirent.h>

requestHandler::requestHandler(Client* ptr) : _ptr(ptr) {

	request.valRead = 0;
	request.exists = true;
	initHeaders();
};

requestHandler::~requestHandler(void){};

void	requestHandler::initHeaders(void) {

	request._headers["Host"] = "";
	request._headers["User-Agent"] = "";
	request._headers["Accept"] = "";
	request._headers["Accept-Language"] = "";
	request._headers["Accept-Encoding"] = "";
	request._headers["Referer"] = "";
	request._headers["Content-Type"] = "";
	request._headers["Content-Length"] = "";
	request._headers["Origin"] = "";
	request._headers["DNT"] = "";
	request._headers["Connection"] = "";
	request._headers["Upgrade-Insecure-Request"] = "";
	request._headers["Sec-Fetch-Dest"] = "";
	request._headers["Sec-Fetch-Mode"] = "";
	request._headers["Sec-Fetch-Site"] = "";
	request._headers["Sec-Fetch-User"] = "";
	request._headers["Pragma"] = "";
	request._headers["Cache-Control"] = "";
}

Request	requestHandler::readRequest(void) {

	char	buf[1024];

    request.connect_fd = _ptr->get_socket();
	request.valRead = read(request.connect_fd, buf, 1023);
	if (request.valRead < 0) {
		perror("reading from connection socket failed");
		close(request.connect_fd);
		exit(1);
	}
	buf[request.valRead] = '\0';
	request._rawRequest.append(buf);
	parseRequest();

	return (request);
};

// can be used for directory listing
int		requestHandler::setPath(std::string current_dir, std::string path) {

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
			request.resource.append(tmp);
			closedir(dir);
			return (0);
		}
		entry = readdir(dir);
	}
	if (current_dir.compare("www") == false)
		request.resource.append("www/html/404.html");
	closedir(dir);
	return (1);
}

void	requestHandler::printHeaders(void) {

	std::map<std::string, std::string>::iterator it;

	for (it = request._headers.begin(); it != request._headers.end(); it++) {
		if (it->first.empty())
			std::cout << it->first << ": " << it->second << std::endl;
	}
}

/* Getters */
//! make em return const references, is quicker. Also do more research on references and constness. And getters and setters.

//! Try to understand once more what you are doing by making the getter const, why, and how it affects your program
// std::string	requestHandler::getStatusLine(void) const {
// 	return (_statusLine);
// }

// std::string	requestHandler::getURL(void) const {
// 	return (_URL);
// }

// std::string	requestHandler::getRawRequest(void) const {
// 	return (_rawRequest);
// }

// std::string	requestHandler::getBody(void) const {
// 	return (_body);
// }

// std::string	requestHandler::getPath(void) const {
// 	return (_path);
// }

// std::map<std::string, std::string>	requestHandler::getHeaders(void) const {
// 	return (_headers);
// }

/**	THE INSERTION OPERATOR OVERLOAAAAAD */

void	printSocketInfo(std::ostream& out, Request const &obj);
void	printRequestDetails(std::ostream& out, Request const &obj);


requestHandler&	requestHandler::operator=( requestHandler &obj) {
	_ptr = obj.getClient();
	return *(this);
}

std::ostream&   operator<<(std::ostream& out, Request const &obj) {

	out << CYAN "HTTP Request\n" RST;
	out << "  " U_WHT "General:\n" RST;
	out << "    Method: " << obj.method << "\n";
	out << "    URL: " << obj._URL << "\n";
	out << "  " U_WHT "Parsing variables:\n" RST;
	out << "    exists: " << (obj.exists? "true" : "false") << "\n";
	out << "    resource: " << obj.resource << "\n";
	out << "    extension: " << obj.extension << "\n";
	out << "    _path: " << obj._path << "\n";

	// printSocketInfo(out, obj);
	// printRequestDetails(out, obj);
	return (out);
}

void	printSocketInfo(std::ostream& out, Request const &obj) {

	out << "  " U_WHT "Socket information:\n" RST;
	out << "    valRead: " << obj.valRead << "\n";
	out << "    connect_fd: " << obj.connect_fd << "\n";
}

void	printRequestDetails(std::ostream& out, Request const &obj) {

	out << "  " U_WHT "Request details:\n" RST;
	out << "    _statusLine: " << obj._statusLine << "\n";
	out << "    _headers:";

	std::map<std::string, std::string>::const_iterator it = obj._headers.begin();

	for (; it != obj._headers.end(); it++) {
		out << (it == obj._headers.begin() ? " " : "              ") << it->first << ": " << (it->second.empty() ? "<empty>" : it->second) << "\n";
	}
	// out << "\n    _body: " << (obj.getBody().empty() ? "<empty>" : obj.getBody()) << "\n";
	out << "\n    _body: " << (obj._body.empty() ? "<empty>" : obj._body) << "\n";
}
