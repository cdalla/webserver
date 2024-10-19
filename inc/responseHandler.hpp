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

        Client*             _ptr;

        void		determineType(Request &request);
		bool		makeStatusLine(Request &r);
		void		fillBody(bool status);
		Response	Debug(Request &request);

		std::ifstream	_ifs;

    public:

        responseHandler(Client *ptr);
        ~responseHandler(void);

		responseHandler& operator=( responseHandler &obj);

		Response response;

        Response	create(Request &request);

		int		statusCode;
		bool	binaryMode;

		Client*	getClient(void) {return this->_ptr;};
};

#endif
