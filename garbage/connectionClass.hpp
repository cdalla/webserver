#ifndef CONNECTIONCLASS_HPP
# define CONNECTIONSCLASS_CLASS

#include "serverClass.hpp"


class Connection
{
	
	public:
	
			int socket;
			struct epoll_event ev;
			struct sockaddr_in addr;
			socklen_t *addrLen;
			
			Connection(){};
			~Connection(){};
};

#endif