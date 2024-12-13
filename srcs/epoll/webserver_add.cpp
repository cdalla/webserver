#include "webserver.hpp"

//add fd_handler to vector
void	Webserver::addHandler(Fd_handler *ptr)
{
	if (_handlers.empty() || std::find(_handlers.begin(), _handlers.end(), ptr) == _handlers.end())
		_handlers.push_back(ptr);
}

//add entry in _fds,
void    Webserver::addFdToMap(int fd, Fd_handler *ptr)
{
	_fds[fd] = ptr;
	addHandler(ptr);
}

//return Fd of epoll instance for FILES or CONNECTIONS
int		Webserver::get_EpollFd(int type)
{
	if (type == CONN)
		return (_epollConn);
	return (_epollFile);
}

//checks if a port is already registered
bool Webserver::is_PortInUse(unsigned int port, std::string ip, std::vector<Server>::iterator end)
{
	for (std::vector<Server>::iterator it = _servers.begin(); it != end; ++it)
	{
		if ((*it).getPort() == port)
		{
			return true;
		}
	}
	return false;
}

//checks if fd is already present in the _fds map
bool	Webserver::is_in_map(int fd)
{
	if (_fds.find(fd) != _fds.end())
		return true;
	return false;	
}