#ifndef CLIENT_HPP
# define CLIENT_HPP

// # include "webserver.hpp"
// # include "socket.hpp"
// #include "client.hpp"
# include "server.hpp"
// # include "request.hpp"
// # include "response.hpp"

#include "structs.hpp"
#include <chrono>
// class Server;
// class Request;
// class Response;

#define TIMEOUT 10

class Client: public Socket
{

    private:

		Request        request;
        Response        response;

		char            _req_buff[1024];
        std::string     _resp_string;
        bool            _done;
        
        std::chrono::time_point<std::chrono::steady_clock> _last_activity;

    public:

		Client(Server *server);
		~Client();
        Server*         _server;


		virtual bool	consume(int event_type);
        void            reset_last_activity();
        bool            has_timeout();
};

#endif