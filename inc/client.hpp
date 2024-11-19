#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "server.hpp"
# include "structs.hpp"
# include "cgi.hpp" 

// class Server;
// class Request;
// class Response;

class Client: public Fd_handler
{

    private:

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
        Response        response;
        std::string    response2;
        Request        request;
		std::string		status;


		virtual bool	consume(int event_type);
        virtual void            input(void);
        virtual void            output(void);

};

#endif