#ifndef REQUESTHANDLER_HPP
# define REQUESTHANDLER_HPP

#include "structs.hpp"

class Client;

class requestHandler {

    private:

        Client*          _ptr;

    public:

    	requestHandler(Client* ptr);
        ~requestHandler(void);

		requestHandler& operator=( requestHandler &obj);

		Request		request;

		void		initHeaders(void);
        Request		readRequest(void);
		void		parseRequest(void);
		void		printHeaders(void);

		Client*	getClient(void) {return this->_ptr;};

	private:

		void		parseStatusLine(void);
		void		parseHeaders(void);
		std::string	getKey(const std::string &line);
		std::string	getValue(const std::string &line);
		int			setPath(std::string current_dir, std::string path);
};

std::ostream&   operator<<(std::ostream& out, Request const &obj);

#endif