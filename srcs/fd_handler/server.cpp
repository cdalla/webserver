#include "server.hpp"
#include "WebservException.hpp"
#include "client.hpp"

Server::Server(VirtualServer &configStruct, Webserver* ptr) : _config(configStruct), _main(ptr)
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
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0)
		throw WebservException("Failed to create new socket: " + std::string(strerror(errno)));
	
	int sockoption = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &sockoption, sizeof(sockoption)) < 0)
		throw WebservException("Failed to setsockopt: " + std::string(strerror(errno)));
	make_socket_non_blocking(_fd);

	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);
	_addrLen = sizeof(_address);
	if (bind(_fd, reinterpret_cast<sockaddr*>(&_address), _addrLen) < 0)
		throw WebservException("Failed to bind socket: " + std::string(strerror(errno)));
	
	if (listen(_fd, MAX_CONNECTIONS) < 0)
		throw WebservException("Failed to listen on socket: " + std::string(strerror(errno)));
	
	std::cout << "server listening on port " << _port << "!" << std::endl;
}


void Server::input()
{
	Fd_handler *client = new Client(this, _main);
	client->set_fd(accept(_fd, reinterpret_cast<sockaddr*>(&_address), &_addrLen));
	if (client->get_fd() == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) 
		{
			print_error("Cannot accept new connection");
			delete client;
			return;
		}
		throw WebservException("Failed to accept new connection: " + std::string(strerror(errno)));
	}
	if (!_main->is_in_map(client->get_fd()))
	{
    	make_socket_non_blocking(client->get_fd());
		_main->addFdToPoll(client->get_fd(), _main->get_EpollFd(CONN), EPOLLIN);
		_main->addFdToMap(client->get_fd(), client);
	}
	else
		delete client;
	std::cout << "added client" << std::endl;
}

void Server::output()
{
	return;
}
VirtualServer&	Server::get_config() const
{
	return _config;
}