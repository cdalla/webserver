#include "server.hpp"

Server::Server()
{
    //fill info to set the socket
    //fill with conf info
    createSocket();
}

int Server::get_socket() const
{
	return this->_socket;
}

struct epoll_event * Server::get_event()
{
	return &(this->_event);
}

void Server::createSocket()
{
    	//CREATE SOCKET
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (!_socket)
	{
		std::cerr << "Failed to create socket!" << std::endl;
		//error or exception, change return type funct
	}
	
	int sockoption = 1;
	//SET NON BLOCKING AND REUSABLE ADDR
	//SOL_SOCKET used for option protocol indipendent, reuse address as 1 = true
	setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &sockoption, sizeof(sockoption)); //check error
	make_socket_non_blocking(_socket);

	//BIND TO ADDRESS AND PORT
	_address.sin_family = AF_INET; //ipv4
	_address.sin_addr.s_addr = INADDR_ANY; //listen on any address
	_address.sin_port = htons(_port); // convert port value
	if (bind(_socket, (struct sockaddr*)&_address, sizeof(_address)) < 0)
	{
		std::cerr << "Failed to bind socket!" << std::endl;
		close(_socket);
		//error or exception, change return type funct
	}
	
	//LISTEN FOR INCOMING CONNECTIONS
	if (listen(_socket, MAX_CONNECTIONS) < 0)
	{
		std::cerr << "Failed to listen on socket!" << std::endl;
		//error or exception, change return type function
	}
}