#ifndef	SERVERCLASS_HPP
# define SERVERCLASS_HPP

#include <vector>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

class Server
{
	public:
	
	struct sockaddr_in address;
	struct epoll_event event;
	int socket;
	
	unsigned int port;
	
		Server(){};
		~Server(){};
		
};

#endif