#ifndef SERVER_HPP
# define SERVER_HPP

class Server
{
	private:    
        
        struct sockaddr_in 		address;
	    struct epoll_event 		event;
	    int 					socket;
	    unsigned int 			port;
	
    public:
	
    	Server(){};
		~Server() = default;

        void createSocket();

};

#endif