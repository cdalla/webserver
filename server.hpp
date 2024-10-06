#ifndef SERVER_H
# define SERVER_H

#include "webserver.hpp"
#include "socket.hpp"

class Server : public Socket 
{
	private:    
    
	    unsigned int 			_port;
	
	public:
	
    	Server();
		~Server() = default;

        void 					createSocket();
		virtual bool    consume(int event_type);

};

#endif