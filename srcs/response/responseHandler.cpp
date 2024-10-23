#include "responseHandler.hpp"
#include "colours.hpp"
#include "utils.hpp"

responseHandler::responseHandler(Client *ptr, VirtualServer config) : _ptr(ptr), config(config) {

	_response.statusLine.append("HTTP/1.1 ");
	_response.contentLength.append("Content-Length: ");
	_response.contentType.append("Content-Type: ");
	_binaryMode = false;
};

responseHandler::~responseHandler(void) {};

Response	responseHandler::_GET(Request &request) {
	return (_response);
}

Response	responseHandler::_POST(Request &request) {
	return (_response);
}

Response	responseHandler::_DELETE(Request &request) {
	return (_response);
}

Response	responseHandler::create(Request &request) {

	_determineType(request); // set "contentType" and "binaryMode"
	if (!request.extension.compare(".debug"))
		return (_debug(request));

	response = methods[request.method];
	
	_fillBody(_makeStatusLine(request));

	return (_response);
}

void	responseHandler::_determineType(Request &r) {

	if (!r.extension.compare(".jpg") || !r.extension.compare(".jpeg")) {
		_response.contentType.append("image/jpeg");
		_binaryMode = true;
	}
	else if (!r.extension.compare(".mp4")) {
		_response.contentType.append("audio/mp4");
		_binaryMode = true;
	}
	else if (!r.extension.compare(".css"))
		_response.contentType.append("text/css");
	else if (!r.extension.compare(".js"))
		_response.contentType.append("application/javascript");
	else if (!r.extension.compare(".debug"))
		_response.contentType.append("text/plain");
	else //if (!r.extension.compare(".html"))
		_response.contentType.append("text/html");
	_response.contentType.append("\r\n");
}

bool	responseHandler::_makeStatusLine(Request &r) {

	if (_binaryMode)
		_ifs.open(r.resource.c_str(), std::ifstream::in | std::ios::binary);
	else
		_ifs.open(r.resource.c_str(), std::ifstream::in);
	if (_ifs.is_open() == true) {
		if (r.exists)
			_response.statusLine.append("200 OK");
		else
			_response.statusLine.append("404 Not Found");
	}
	else {
		_response.statusLine.append("500 Internal Server Error\r\n");
		return (true);
	}

	return (false);
}

void	responseHandler::_fillBody(bool status) {

	std::streampos	size;
        
	status = status;
	_ifs.seekg(0, std::ios::end); // Set the file pointer within the input stream. We want to start at 0 (cause we want to use this to get the length of the file) and set it to the end of the file - which is represented by std::ios::end (which is an enumerator))
	size = _ifs.tellg(); // Tellg will return the position of the file pointer. And we just set it to the end so the location of the file pointer will be the last element, ergo the total filesize.
	_ifs.seekg(0, std::ios::beg); // Set that bad boy to the start of the input stream again
	_response.entityBody.resize(size); // Give entityBody enough space that we can read straight into entityBody. This is what ChatGPT did. I'm gonna check later if actually necessary or if can also read into std::string and append that to entityBody. But I think maybe that's not possible because the whole problem with serving images was that I was storing the binary data into std::string but wait entityBody is also std::string. Maybe so we don't have to read line by line?
	
	_ifs.read(&(_response.entityBody[0]), size);
	_ifs.close();
	_response.contentLength.append(std::to_string(_response.entityBody.length()));
	_response.statusLine.append("\r\n");
	_response.contentLength.append("\r\n\r\n");
}

Response	responseHandler::_debug(Request &request) {

	_response.statusLine.append("200 OK");
	_response.entityBody.append("This is the raw request: \n");
	_response.entityBody.append(request._rawRequest);
	_response.contentLength.append(std::to_string(_response.entityBody.length()));
	_response.contentLength.append("\r\n\r\n");
	_response.statusLine.append("\r\n");

	return (_response);
}

Location	responseHandler::_getLocation(std::string const &path) {
	Location	longestMatch;
	std::string	prefix = get_URI_prefix(path);

	for (std::vector<Location>::iterator it = config.locations.begin(); it != config.locations.end(); it++) {
		std::string::const_iterator pathIter = path.begin();
		std::string::iterator locationIter = (*it).path.begin();	
	}
}

/* 	GETTERS & SETTERS */

Client*		responseHandler::getClient(void) const {
	return (this->_ptr);
}

int			responseHandler::getStatusCode(void) const {
	return (this->_statusCode);
}

bool		responseHandler::getBinaryMode(void) const {
	return (this->_binaryMode);
}

Response	responseHandler::getResponse(void) const {
	return (this->_response);
}

/* 	OPERATOR OVERLOADS */

std::ostream&	operator<<(std::ostream &out, responseHandler const &obj) {
	out << CYAN << "responseHandler" << RST << std::endl;
	out << "  " << U_WHT << "response:" << RST << std::endl;
	out << obj.getResponse() << std::endl;
	out << "  " << U_WHT << "Parsing variables:" << RST << std::endl;
	out << "    binaryMode: " << (obj.getBinaryMode() ? "true" : "false") << std::endl;

	return (out);
}

std::ostream&	operator<<(std::ostream& out, Response const &obj) {

	out << "    statusLine: " << obj.statusLine;
	out << "    contentType: " << obj.contentType;
	out << "    contentLength: " << obj.contentLength;
	// not going to print out entityBody as default cause that's gonna be super annoying data in binary mode
	return (out);
}
