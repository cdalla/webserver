#include "client.hpp"


Client::Client()
{
    _addrLen = (socklen_t *)sizeof(_address); //client address
}

int 					Client::get_socket() const
{
    return this->_socket;
}

struct sockaddr_in *	Client::get_address()
{
    return &(this->_address);
}

socklen_t*			    Client::get_addLen() const
{
    return this->_addrLen;
}

struct epoll_event *	Client::get_event()
{
    return &(this->_event);
}

void                    Client::set_socket(int socket)
{
    this->_socket = socket;
}