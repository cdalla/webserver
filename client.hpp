#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webserver.hpp"
# include "socket.hpp"
# include "server.hpp"
# include "request.hpp"
# include "response.hpp"

class Server;
class Request;
class Response;

class Client: public Socket
{

    private:

		Request*         _req_handl;
        Response*        _resp_handl;

		char            _req_buff[1024];
        std::string     _resp_string;
        bool            _done;

        Server*         _server;


    public:
			
		Client(Server *server);
		~Client();

		virtual bool    		consume(int event_type);

};

#endif