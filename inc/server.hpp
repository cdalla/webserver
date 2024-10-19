#ifndef SERVER_HPP
# define SERVER_HPP

// #include "webserver.hpp"
// #include "server.hpp"
#include "socket.hpp"
#include "structs.hpp"

class Server : public Socket 
{
	private:    
    
	    unsigned int 			_port;
		VirtualServer			_config;
	
	public:
	
    	Server(VirtualServer configStruct);
		~Server() = default;

        void 					createSocket();
		virtual bool    consume(int event_type);

};

#endif