#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "webserver.hpp"
# include "client.hpp"
#include <string>
#include <fstream>
#include <iostream>

class Client;
class Request;

class Response
{
    
    private:

        Client*             _ptr;

        void	determineType(Request &request);
		void	makeStatusLine(Request &r);
		void	fillBody(void);
		void	Debug(Request &request);

		std::ifstream	_ifs;

    public:

        Response(Client *ptr);
        ~Response() = default;

        std::string function(void);
        		void	create(Request &request);

		// std::string	getBody(void) const;

		int		statusCode;
		bool	binaryMode;
	
		std::string	statusLine;
		std::string	contentType;
		std::string	contentLength;
		std::string	entityBody;

};

#endif
