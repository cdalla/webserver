#ifndef SERVER_HPP
# define SERVER_HPP

#include "webserver.hpp"
// #include "server.hpp"
#include "fd_handler.hpp"
#include "structs.hpp"

class Webserver;

class Server : public Fd_handler
{
	private:    
    
	    unsigned int 			_port;
		VirtualServer			_config;
		Webserver*				_main;

        socklen_t               _addrLen;
        struct sockaddr_in 		_address;

	
	public:
	
    	Server(VirtualServer configStruct, Webserver* ptr);
		~Server() = default;
		
        void 					createSocket();
		virtual bool    		consume(int event_type);
		virtual void            input(void);
        virtual void            output(void);
		VirtualServer			get_config() const;


};

#endif