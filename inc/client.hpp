#ifndef CLIENT_HPP
# define CLIENT_HPP

// # include "webserver.hpp"
// # include "socket.hpp"
// #include "client.hpp"
# include "server.hpp"
// # include "request.hpp"
// # include "response.hpp"

#include "structs.hpp"

// class Server;
// class Request;
// class Response;

class Client: public Fd_handler
{

    private:

		Request         request;
        Response        response;

		char            _req_buff[MAX_BUFF];
        std::string     _resp_string;
        bool            _done;

    public:

		Client(Server *server, Webserver *main);
		~Client();
        Server*         server;
        Webserver*      main;
        std::string     file_content;
        std::string     cgi_result;


		virtual bool	consume(int event_type);

};

#endif