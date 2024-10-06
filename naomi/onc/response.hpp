#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <string>
#include <fstream>
#include <iostream>

class Request;

class Response {
	public:
		Response(void);
		~Response(void);

		void	create(Request &request);

		// std::string	getBody(void) const;

		int		statusCode;
		bool	binaryMode;
	
		std::string	statusLine;
		std::string	contentType;
		std::string	contentLength;
		std::string	entityBody;
	
	private:
		void	determineType(Request &request);
		void	makeStatusLine(Request &r);
		void	fillBody(void);
		void	Debug(Request &request);

		std::ifstream	_ifs;
};

std::ostream&	operator<<(std::ostream& out, Response const &obj);

#endif