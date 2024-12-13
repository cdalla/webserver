#include "server.hpp"
#include "WebservException.hpp"
#include "client.hpp"
#include <sstream>

Server::Server(VirtualServer &configStruct, Webserver* ptr, Config* conf) : _conf(configStruct), _main(ptr), _config(conf)
{
	_port = _conf.listen;
	_ip = _conf.ip;
}

/*	
	CREATE SOCKET
	SET NON BLOCKING AND REUSABLE ADDR
	SOL_SOCKET used for option protocol indipendent, reuse address as 1 = true
	BIND TO ADDRESS AND PORT
	LISTEN FOR INCOMING CONNECTIONS
*/

uint32_t ipStringToDecimal( const std::string ip_address ) {
	uint32_t result = 0;
	std::istringstream ip_stream(ip_address);
	std::string octet;
	int shift = 24;
	int octet_count = 0;
	while (std::getline(ip_stream, octet, '.')) {
		uint32_t octetValue = static_cast<uint32_t>(std::stoi(octet));
		if (octetValue > 255) {
			throw WebservException("Invalid IP address: " + ip_address);
		}
		result |= (octetValue << shift);
        shift -= 8;
		++octet_count;
	}
	if (octet_count != 4 && result != 0) {
		throw WebservException("Invalid IP address: " + ip_address);
	}
	return result;
}

void Server::createSocket()
{
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0)
		throw WebservException("Failed to create new socket: " + std::string(strerror(errno)));
	
	int sockoption = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &sockoption, sizeof(sockoption)) < 0)
		throw WebservException("Failed to setsockopt: " + std::string(strerror(errno)));
	make_socket_non_blocking(_fd);
	uint32_t ip;
	if (!_ip.empty())
		ip = ipStringToDecimal(_ip);
	else
		ip = INADDR_ANY;
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = htonl(ip);
	_address.sin_port = htons(_port);
	_addrLen = sizeof(_address);
	if (bind(_fd, (sockaddr*)(&_address), _addrLen) < 0)
	{
		throw WebservException("Failed to bind socket: " + std::string(strerror(errno)));
	}
	
	if (listen(_fd, MAX_CONNECTIONS) < 0)
		throw WebservException("Failed to listen on socket: " + std::string(strerror(errno)));
	
	std::cout << "server listening on port " << _port << ", fd: " << _fd << "!" << std::endl;
}


void Server::input()
{
	Fd_handler *client = new Client(this, _main, _config);
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
	{

		print_error("client already present");
		delete client;
	}
	std::cout << "added client with fd: " << client->get_fd() <<   std::endl;
}

void Server::output()
{
	return;
}

VirtualServer&	Server::get_config() const
{
	return _conf;
}

unsigned int Server::getPort()
{
	return this->_port;
}

std::string Server::getIp()
{
	return this->_ip;
}