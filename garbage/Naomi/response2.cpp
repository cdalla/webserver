#include "response2.hpp"
#include "request.hpp"
#include "colours.hpp"

Response::Response(void) {

	statusLine.append("HTTP/1.1 ");
	contentLength.append("Content-Length: ");
	contentType.append("Content-Type: ");
	binaryMode = false;
};

Response::~Response(void) {};

void	Response::create(Request &request) {
	determineType(request); // set "contentType" and "binaryMode"
	if (!request.extension.compare(".debug"))
		return (Debug(request));
	makeStatusLine(request);
	fillBody();
}

void	Response::determineType(Request &r) {
	if (!r.extension.compare(".jpg") || !r.extension.compare(".jpeg"))
	{
		contentType.append("image/jpeg");
		binaryMode = true;
	}
	else if (!r.extension.compare(".mp4")) {
		contentType.append("audio/mp4");
		binaryMode = true;
	}
	else if (!r.extension.compare(".css"))
		contentType.append("text/css");
	else if (!r.extension.compare(".js"))
		contentType.append("application/javascript");
	else if (!r.extension.compare(".debug"))
		contentType.append("text/plain");
	else //if (!r.extension.compare(".html"))
		contentType.append("text/html");
	contentType.append("\r\n");
}

void	Response::makeStatusLine(Request &r) {
	if (binaryMode)
		_ifs.open(r.resource.c_str(), std::ifstream::in | std::ios::binary);
	else
		_ifs.open(r.resource.c_str(), std::ifstream::in);
	if (_ifs.is_open() == true)
	{
		if (r.exists)
			statusLine.append("200 OK");
		else
			statusLine.append("404 Not Found");
	}
	else {
		statusLine.append("500 Internal Server Error\r\n");
		return ;
	}
}

void	Response::fillBody(void) {

	std::streampos	size;

	_ifs.seekg(0, std::ios::end); // Set the file pointer within the input stream. We want to start at 0 (cause we want to use this to get the length of the file) and set it to the end of the file - which is represented by std::ios::end (which is an enumerator))
	size = _ifs.tellg(); // Tellg will return the position of the file pointer. And we just set it to the end so the location of the file pointer will be the last element, ergo the total filesize.
	_ifs.seekg(0, std::ios::beg); // Set that bad boy to the start of the input stream again
	entityBody.resize(size); // Give entityBody enough space that we can read straight into entityBody. This is what ChatGPT did. I'm gonna check later if actually necessary or if can also read into std::string and append that to entityBody. But I think maybe that's not possible because the whole problem with serving images was that I was storing the binary data into std::string but wait entityBody is also std::string. Maybe so we don't have to read line by line?
	_ifs.read(&entityBody[0], size);
	_ifs.close();
	contentLength.append(std::to_string(entityBody.length()));
	contentLength.append("\r\n\r\n");
	statusLine.append("\r\n");
}

void	Response::Debug(Request &request) {
	statusLine.append("200 OK");
	entityBody.append("This is the raw request: \n");
	entityBody.append(request.getRawRequest());
	contentLength.append(std::to_string(entityBody.length()));
	contentLength.append("\r\n\r\n");
	statusLine.append("\r\n");
}

std::ostream&	operator<<(std::ostream& out, Response const &obj) {
	out << CYAN "HTTP Response\n" RST;
	out << "  " U_WHT "General:\n" RST;
	out << "    statusLine: " << obj.statusLine;
	out << "    contentType: " << obj.contentType;
	out << "    contentLength: " << obj.contentLength;
	out << "  " U_WHT "Parsing variables:\n" RST;
	out << "    binaryMode: " << (obj.binaryMode? "true" : "false") << "\n";
	return (out);
}

// std::string	Response::getBody(void) const {
// 	return (_entityBody);
// }
