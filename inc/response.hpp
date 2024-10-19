#ifndef RESPONSE_HPP
# define RESPONSE_HPP

// # include "webserver.hpp"
// # include "client.hpp"
#include <string>
#include <fstream>
#include <iostream>
# include <vector>
# include <map>

class Client;
class Request;

class Response
{
    
    private:

        Client*             _ptr;

        void	determineType(Request &request);
		bool	makeStatusLine(Request &r);
		void	fillBody(bool status);
		void	Debug(Request &request);

		std::ifstream	_ifs;

    public:

        Response(Client *ptr);
        ~Response() = default;

		Response& operator=( Response &obj);

        std::string function(void);
        void	create(Request &request);

		// std::string	getBody(void) const;

		int		statusCode;
		bool	binaryMode;
	
		std::string	statusLine;
		std::string	contentType;
		std::string	contentLength;
		std::string	entityBody;

		Client*	getClient(void) {return this->_ptr;};

};

#endif
