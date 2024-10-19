#include "responseHandler.hpp"
#include "colours.hpp"

responseHandler::responseHandler(Client *ptr) : _ptr(ptr) {

	response.statusLine.append("HTTP/1.1 ");
	response.contentLength.append("Content-Length: ");
	response.contentType.append("Content-Type: ");
	binaryMode = false;
};

responseHandler::~responseHandler(void) {};

Response	responseHandler::create(Request &request) {

	determineType(request); // set "contentType" and "binaryMode"
	if (!request.extension.compare(".debug"))
		return (Debug(request));
	fillBody(makeStatusLine(request));

	return (response);
}

void	responseHandler::determineType(Request &r) {

	if (!r.extension.compare(".jpg") || !r.extension.compare(".jpeg")) {
		response.contentType.append("image/jpeg");
		binaryMode = true;
	}
	else if (!r.extension.compare(".mp4")) {
		response.contentType.append("audio/mp4");
		binaryMode = true;
	}
	else if (!r.extension.compare(".css"))
		response.contentType.append("text/css");
	else if (!r.extension.compare(".js"))
		response.contentType.append("application/javascript");
	else if (!r.extension.compare(".debug"))
		response.contentType.append("text/plain");
	else //if (!r.extension.compare(".html"))
		response.contentType.append("text/html");
	response.contentType.append("\r\n");
}

bool	responseHandler::makeStatusLine(Request &r) {

	if (binaryMode)
		_ifs.open(r.resource.c_str(), std::ifstream::in | std::ios::binary);
	else
		_ifs.open(r.resource.c_str(), std::ifstream::in);
	if (_ifs.is_open() == true) {
		if (r.exists)
			response.statusLine.append("200 OK");
		else
			response.statusLine.append("404 Not Found");
	}
	else {
		response.statusLine.append("500 Internal Server Error\r\n");
		return (true);
	}

	return (false);
}

void	responseHandler::fillBody(bool status) {

	std::streampos	size;
        
	status = status;
	_ifs.seekg(0, std::ios::end); // Set the file pointer within the input stream. We want to start at 0 (cause we want to use this to get the length of the file) and set it to the end of the file - which is represented by std::ios::end (which is an enumerator))
	size = _ifs.tellg(); // Tellg will return the position of the file pointer. And we just set it to the end so the location of the file pointer will be the last element, ergo the total filesize.
	_ifs.seekg(0, std::ios::beg); // Set that bad boy to the start of the input stream again
	response.entityBody.resize(size); // Give entityBody enough space that we can read straight into entityBody. This is what ChatGPT did. I'm gonna check later if actually necessary or if can also read into std::string and append that to entityBody. But I think maybe that's not possible because the whole problem with serving images was that I was storing the binary data into std::string but wait entityBody is also std::string. Maybe so we don't have to read line by line?
	
	_ifs.read(&(response.entityBody[0]), size);
	_ifs.close();
	response.contentLength.append(std::to_string(response.entityBody.length()));
	response.statusLine.append("\r\n");
	response.contentLength.append("\r\n\r\n");
}

Response	responseHandler::Debug(Request &request) {

	response.statusLine.append("200 OK");
	response.entityBody.append("This is the raw request: \n");
	response.entityBody.append(request._rawRequest);
	response.contentLength.append(std::to_string(response.entityBody.length()));
	response.contentLength.append("\r\n\r\n");
	response.statusLine.append("\r\n");

	return (response);
}

responseHandler&	responseHandler::operator=( responseHandler &obj) {

	_ptr = obj.getClient();

	return *(this);
}

std::ostream&	operator<<(std::ostream& out, Response const &obj) {

	out << CYAN "HTTP Response\n" RST;
	out << "  " U_WHT "General:\n" RST;
	out << "    statusLine: " << obj.statusLine;
	out << "    contentType: " << obj.contentType;
	out << "    contentLength: " << obj.contentLength;
	// out << "  " U_WHT "Parsing variables:\n" RST;
	// out << "    binaryMode: " << (obj.binaryMode? "true" : "false") << "\n";

	return (out);
}
