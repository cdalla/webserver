#ifndef RESPONSEHANDLER_HPP
# define RESPONSEHANDLER_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>

#include "structs.hpp"

class Client;

class responseHandler {

    private:

		Client*			_ptr;

		Response		_GET(Request &request);
		Response		_POST(Request &request);
		Response		_DELETE(Request &request);
        void			_determineType(Request &request);
		bool			_makeStatusLine(Request &r);
		void			_fillBody(bool status);
		Response		_debug(Request &request);
		
		int				_statusCode;
		bool			_binaryMode;
		std::ifstream	_ifs;
		Response		_response;

    public:

        responseHandler(Client *ptr);
		// responseHandler(const responseHandler &src);
        ~responseHandler(void);

		// responseHandler& operator=(responseHandler &src);

        Response	create(Request &request);

		Client*		getClient(void) const;
		int			getStatusCode(void) const;
		bool		getBinaryMode(void) const;
		Response	getResponse(void) const;
};

std::ostream&	operator<<(std::ostream& out, responseHandler const &obj);
std::ostream&	operator<<(std::ostream& out, Response const &obj);

#endif
