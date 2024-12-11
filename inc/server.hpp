#ifndef SERVER_HPP
# define SERVER_HPP

#include "webserver.hpp"
// #include "server.hpp"
#include "fd_handler.hpp"
#include "structs.hpp"
#include "config.hpp"

class Webserver;

class Server : public Fd_handler
{
	private:    
    
	    unsigned int 			_port;
		std::string				_ip;
		VirtualServer			&_config;
		Webserver*				_main;
		Config*					_configuration;
        socklen_t               _addrLen;
        struct sockaddr_in 		_address;

	
	public:
	
    	Server(VirtualServer &configStruct, Webserver* ptr, Config* conf);
		~Server() = default;
		
        void 					createSocket();
		virtual void            input(void);
        virtual void            output(void);
		virtual void			hangup(void){return;}
		VirtualServer&			get_config() const;


};

#endif