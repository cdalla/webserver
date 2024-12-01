#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "server.hpp"
# include "structs.hpp"
# include "cgi.hpp" 
# include "requestParser.hpp"

// class Server;
// class Request;
// class Response;

class Client: public Fd_handler
{

    private:

		char            _req_buff[MAX_BUFF];
        std::string     _resp_string;
        bool            _done;
        ssize_t         _total_bytes_read;

    public:

		Client(Server *server, Webserver *main);
		~Client();
        Server*         server;
        Webserver*      main;
        std::string     file_content;
        std::string     cgi_result;
        Response        response;
        std::string     response2;
        Request         request;
        RequestParser*  parser;
		std::string		status;


        virtual void            input(void);
        virtual void            output(void);
		virtual void			hangup(void){return;}

};

#endif