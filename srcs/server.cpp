#include "server.hpp"

Server::Server(VirtualServer configStruct) : _config(configStruct)
{
    //fill info to set the socket
    //fill with conf info
	_port = _config.listen;
}

/*	
	CREATE SOCKET
	SET NON BLOCKING AND REUSABLE ADDR
	SOL_SOCKET used for option protocol indipendent, reuse address as 1 = true
	BIND TO ADDRESS AND PORT
	LISTEN FOR INCOMING CONNECTIONS
*/
void Server::createSocket()
{
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0)
	{
		//FATAL
		std::cerr << "Failed to create socket!" << std::endl;
		//error or exception, change return type funct
	}
	
	int sockoption = 1;
	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &sockoption, sizeof(sockoption)) < 0)
	{
		//FATAL
		std::cerr << "Failed to setsockopt!" << std::endl; 
	} //check error
	make_socket_non_blocking(_socket);

	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);
	_addrLen = sizeof(_address);
	if (bind(_socket, reinterpret_cast<sockaddr*>(&_address), _addrLen) < 0)
	{
		//FATAL
		std::cerr << "Failed to bind socket!" << std::endl;
		close(_socket);
		//error or exception, change return type funct
	}
	
	if (listen(_socket, MAX_CONNECTIONS) < 0)
	{
		//FATAL
		std::cerr << "Failed to listen on socket!" << std::endl;
		//error or exception, change return type function
	}
	std::cout << "ok on port " << _socket << "!" << std::endl;
}




bool    Server::consume(int event_type)
{
	(void)event_type;
	return false;
}