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
	int socket;
	struct epoll_event event;
	
	unsigned int port;
	
		Server(){};
		~Server(){};
		
};
