#ifndef SOCKET_HPP
# define SOCKET_HPP

#include "webserver.hpp"

class Socket
{
    protected:

        struct sockaddr_in 		_address;
        socklen_t *             _addrLen;
	    struct epoll_event 		_event;
	    int 					_socket;

    public:

        Socket();
        ~Socket() = default;

        struct sockaddr_in * 	get_address(){ return &(this->_address);}
        socklen_t * 			get_addLen() const {return (socklen_t *)sizeof(this->_address);};
        struct epoll_event *	get_event() {return &(this->_event);}
        int 					get_socket() const {return this->_socket;}
        void                    set_socket(int socket) {this->_socket = socket;}

        virtual bool            consume(int event_type){}; 

};


#endif