#include "webserver.hpp"
#include "client.hpp"

/*
	CREATE EPOLL INSTANCE
	ADD ALL SERVER TO EPOLL AND TO MAP CONTAINER

	*from linux 2.6.8 parameter size on epoll_create is ignored because 
	 epoll data struct is dinamically resized
*/
void    Webserver::create_Epoll()
{
    _epollFd = epoll_create(1); 
	if (_epollFd == -1)
		throw WebservException("Failed to create epoll instance: " + std::string(strerror(errno)));
    
	std::vector<Server>::iterator it = _servers.begin();
	for (; it != _servers.end(); ++it)
	{
        addFdToPoll((*it).get_socket());
		_fds[(*it).get_socket()] = &(*it);
    }
}

/*
	ADD NEW SOCKET TO EPOLL INSTANCE
	ACCEPT CONNECTION AND ADD IT TO THE FDS MAP
*/
void    Webserver::addClient(int fd, Server *server)
{
	Socket *client = new Client(server);
	client->set_socket(accept(fd, reinterpret_cast<sockaddr*>(&server->_address), &server->_addrLen));
	if (client->get_socket() == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) 
		{
			print_error("Cannot accept new connection");
			delete client;
			return;
		}
		throw WebservException("Failed to accept new connection: " + std::string(strerror(errno)));
	}
	if (_fds.find(client->get_socket()) == _fds.end())
	{
    	make_socket_non_blocking(client->get_socket());
		addFdToPoll(client->get_socket());
		_fds[client->get_socket()] = client;
	}
	else
		delete client;
}

/*
	SET EVENT FOR THE FD
	ADD IT TO THE EPOLL INSTANCE
*/
void    Webserver::addFdToPoll(int fd) 
{
	epoll_event event{};
	event.events = EPOLLIN;
	event.data.fd = fd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
		throw WebservException("Failed to add socket to epoll: " + std::string(strerror(errno)));
}

void	Webserver::change_event(int fd)
{
	epoll_event event{};
	event.events = EPOLLOUT;
	event.data.fd = fd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &event) == -1)
		throw WebservException("Failed to modify epoll event: " + std::string(strerror(errno)));
}

/*
	REMOVE FD FROM EPOLL INSTANCE
	REMOVE FROM FDS MAP AND DELETE HIS DATA
*/
void    Webserver::removeFd(int fd)
{
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
		throw WebservException("Failed to remove socket from epoll: " + std::string(strerror(errno)));
	if (dynamic_cast<Client *>(_fds[fd]))
		delete (_fds[fd]);
	_fds.erase(fd);
	close (fd);
}