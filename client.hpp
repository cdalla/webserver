#ifndef CLIENT_HPP
# define CLIENT_HPP

class Client
{

    private:

			struct epoll_event      event;
			struct sockaddr_in      address;
    		int                     socket;
			socklen_t               *addrLen;


    
    public:
			
			Client(){};
			~Client(){};

};

#endif