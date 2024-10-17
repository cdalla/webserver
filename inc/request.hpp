#ifndef REQUEST_HPP
# define REQUEST_HPP

// # include "webserver.hpp"
# include "response.hpp"
// # include "client.hpp"

class Client;

class Request
{

    private:

        Client*          _ptr;

    public:

        Request(Client* ptr);
        ~Request() = default;

		Request& operator=( Request &obj);

        
        // bool function(char *buff);
        void		readRequest(void);
		void		parseRequest(void);
		void		printHeaders(void);

		/** Getters  */
		std::string	getStatusLine(void) const;
		std::string	getURL(void) const;
		std::string	getRawRequest(void) const;
		std::string	getBody(void) const;
		std::string	getPath(void) const;
		std::map<std::string, std::string>	getHeaders(void) const;

		/** Init functions */
		void		initHeaders(void);

		int			valRead;
		int			connect_fd;
		bool		exists;
	
		std::string	method;
		std::string	resource;
		std::string	extension;

		Client*	getClient(void) {return this->_ptr;};


	private:

		void		parseStatusLine(void);
		void		parseHeaders(void);
		std::string	getKey(const std::string &line);
		std::string	getValue(const std::string &line);
		int			setPath(std::string current_dir, std::string path);
	
		std::string		_statusLine;
		std::string		_URL;
		std::string		_path;
		std::string		_body;
		std::string		_rawRequest;
		std::map<std::string, std::string>	_headers;

};

std::ostream&   operator<<(std::ostream& out, Request const &obj);


#endif