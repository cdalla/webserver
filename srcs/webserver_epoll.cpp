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
    _epollConn = epoll_create(1); 
	if (_epollConn == -1)
		throw WebservException("Failed to create epoll instance: " + std::string(strerror(errno)));
    
	std::vector<Server>::iterator it = _servers.begin();
	for (; it != _servers.end(); ++it)
	{
        addFdToPoll((*it).get_fd(), _epollConn, EPOLLIN);
		_fds[(*it).get_fd()] = &(*it);
    }
	_epollFile = epoll_create(1);
	if (_epollFile == -1)
		throw WebservException("Failed to create epoll instance: " + std::string(strerror(errno)));

}

/*
	SET EVENT FOR THE FD
	ADD IT TO THE EPOLL INSTANCE
*/
void    Webserver::addFdToPoll(int fd, int epollFd, uint32_t events) 
{
	epoll_event event{};
	event.events = events;
	event.data.fd = fd;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
		throw WebservException("Failed to add socket to epoll: " + std::string(strerror(errno)));
}

void	Webserver::change_event(int fd)
{
	epoll_event event{};
	event.events = EPOLLOUT;
	event.data.fd = fd;
	if (epoll_ctl(_epollConn, EPOLL_CTL_MOD, fd, &event) == -1)
		throw WebservException("Failed to modify epoll event: " + std::string(strerror(errno)));
}

/*
	REMOVE FD FROM EPOLL INSTANCE
	REMOVE FROM FDS MAP AND DELETE HIS DATA
*/
void    Webserver::removeFd(int fd, int type)
{
	std::cout << "deleting fd: " << fd << std::endl;
	if (type == CONN)
	{
		if (epoll_ctl(_epollConn, EPOLL_CTL_DEL, fd, NULL) == -1)
			throw WebservException("Failed to remove socket from epoll: " + std::string(strerror(errno)));
		if (dynamic_cast<Client *>(_fds[fd]))
			delete (_fds[fd]);
	}
	else if (type == FILES)
	{
		if (epoll_ctl(_epollFile, EPOLL_CTL_DEL, fd, NULL) == -1)
			throw WebservException("Failed to remove socket from epoll: " + std::string(strerror(errno)));
		//if (_fds[fd])
			//delete(_fds[fd]);
	}
	_fds.erase(fd);
	close (fd);
}