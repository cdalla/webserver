#include "server.hpp"
#include "WebservException.hpp"

Server::Server(VirtualServer configStruct) : _config(configStruct)
{
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
		throw WebservException("Failed to create new socket: " + std::string(strerror(errno)));
	
	int sockoption = 1;
	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &sockoption, sizeof(sockoption)) < 0)
		throw WebservException("Failed to setsockopt: " + std::string(strerror(errno)));
	make_socket_non_blocking(_socket);

	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);
	_addrLen = sizeof(_address);
	if (bind(_socket, reinterpret_cast<sockaddr*>(&_address), _addrLen) < 0)
		throw WebservException("Failed to bind socket: " + std::string(strerror(errno)));
	
	if (listen(_socket, MAX_CONNECTIONS) < 0)
		throw WebservException("Failed to listen on socket: " + std::string(strerror(errno)));
	
	std::cout << "ok on port " << _socket << "!" << std::endl;
}




bool    Server::consume(int event_type)
{
	(void)event_type;
	return false;
}