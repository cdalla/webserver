#ifndef SOCKET_HPP
# define SOCKET_HPP

// #include "webserver.hpp"
# include <vector>
# include <map>
# include <sys/socket.h>
# include <sys/epoll.h>
# include <arpa/inet.h>
# include <fcntl.h>
# include <iostream>
# include <unistd.h>
# include <cstring>

void make_socket_non_blocking(int socket_fd);
# define MAX_EVENTS 100
# define IN 0
# define OUT 1
# define MAX_SIZE 1024
# define MAX_CONNECTIONS 100

class Socket
{
    protected:

	    struct epoll_event 		_event;
	    int 					_socket;

    public:

        Socket() = default;
        virtual ~Socket() = default;
        socklen_t               _addrLen;
        struct sockaddr_in 		_address;

        struct sockaddr_in * 	get_address(){ return &(this->_address);}
        socklen_t *		        get_addLen() {return &(this->_addrLen);};
        int 					get_socket() const {return this->_socket;}
        void                    set_socket(int socket) {this->_socket = socket;}

        struct epoll_event *	get_event() {return &(this->_event);}
        virtual bool            consume(int event_type){(void)event_type;return false;} 

};


#endif