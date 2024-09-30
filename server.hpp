#ifndef SERVER_HPP
# define SERVER_HPP

#include "webserver.hpp"

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