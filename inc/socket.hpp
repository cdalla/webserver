#ifndef SOCKET_HPP
# define SOCKET_HPP

#include "webserver.hpp"

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