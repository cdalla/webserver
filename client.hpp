#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webserver.hpp"

class Client
{

    private:

			struct epoll_event      _event;
			struct sockaddr_in      _address;
    		int                     _socket;
			socklen_t *             _addrLen;

    public:
			
			Client();
			~Client() = default;

			int 					get_socket() const;
			struct sockaddr_in * 	get_address();
			socklen_t * 			get_addLen() const;
			struct epoll_event *	get_event();
			void					set_socket(int socket);
			

};

#endif