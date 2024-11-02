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

class Client: public Socket
{

    private:




		char            _req_buff[1024];
        std::string     _resp_string;
        bool            _done;

    public:

		Client(Server *server);
		~Client();
        Server*        server;
        std::string    response;
        Request        request;

		virtual bool	consume(int event_type);
};

#endif