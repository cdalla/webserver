#ifndef SERVER_HPP
# define SERVER_HPP

#include "webserver.hpp"

# define MAX_CONNECTIONS 10

class Server {
	private:
        
        struct sockaddr_in 		_address;
	    struct epoll_event 		_event;
	    int 					_socket;
	    unsigned int 			_port;
		VirtualServer			_config; // just for now to try and force my code + carlo's code together without worrying about optimization or logical thinking
	
	public:
	
    	// Server(void);
		Server(VirtualServer configStruct);
		~Server(void);
		// ~Server() = default;

        void 					createSocket();
		int						get_socket() const;
		struct epoll_event *	get_event();

};

#endif