#include "webserver.hpp"
#include "client.hpp"

Webserver::Webserver(std::string default_config) : config(default_config.c_str())
{
	config.parseConfig();
	servers_init();
	create_Epoll();
}

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

/*
	LOOP TO INITIALIZE SERVER SOCKET
*/
void    Webserver::servers_init()
{
	for (std::vector<VirtualServer>::iterator it = config.servers.begin(); it != config.servers.end(); it++) {
		if (!is_PortInUse((*it).listen, (*it).ip, _servers.end()))
			_servers.push_back(Server(*it, this, &config));
	}
	
	for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it)
			(*it).createSocket();
}

bool	Webserver::is_in_map(int fd)
{
	if (_fds.find(fd) != _fds.end())
		return true;
	return false;	
}

void	Webserver::addHandler(Fd_handler *ptr)
{
	if (_handlers.empty() || std::find(_handlers.begin(), _handlers.end(), ptr) == _handlers.end())
	{
		print_msg("adding handler");
		_handlers.push_back(ptr);
	}
}

void    Webserver::addFdToMap(int fd, Fd_handler *ptr)
{
	std::cout << "adding fd to map: " << fd << std::endl;
	_fds[fd] = ptr;
	addHandler(ptr);
}

int		Webserver::get_EpollFd(int type)
{
	if (type == CONN)
		return (_epollConn);
	return (_epollFile);
}

/*
	MAIN PROGRAM LOOP:
		-receive number of event by epoll_wait
		-process the event fds triggered
		-add new client or process IN/OUT data transfer
*/
void	Webserver::run()
{
	//throw WebservException("testing close fd");
    struct epoll_event	events_queue[MAX_EVENTS];
	int readyFds = 0;
	while (true)
	{
		check_timeouts();
		{
			readyFds = epoll_wait(_epollConn, events_queue, MAX_EVENTS, 10);
			if (readyFds == -1)
				throw WebservException("Epoll wait failure: " + std::string(strerror(errno)));
			for (int n = 0; n < readyFds; n++)
			{
            	int eventFd = events_queue[n].data.fd;
				if (events_queue[n].events & EPOLLIN && _fds.find(eventFd) != _fds.end())
					_fds[eventFd]->input();
				else if (events_queue[n].events & EPOLLOUT && _fds.find(eventFd) != _fds.end())
					_fds[eventFd]->output();
				else
					removeFd(eventFd, CONN, 1);
			}
		}
		{
			readyFds = epoll_wait(_epollFile, events_queue, MAX_EVENTS, 10);
			if (readyFds == -1)
				throw WebservException("Epoll wait failure: " + std::string(strerror(errno)));
			for (int n = 0; n < readyFds; n++)
			{
            	int eventFd = events_queue[n].data.fd;
				if (events_queue[n].events & EPOLLIN && _fds.find(eventFd) != _fds.end())
					_fds[eventFd]->input();
				else if (events_queue[n].events & EPOLLOUT && _fds.find(eventFd) != _fds.end())
				{
					print_msg("FILES epoll output");
					_fds[eventFd]->output();
				}
				else if (events_queue[n].events & EPOLLHUP && _fds.find(eventFd) != _fds.end())
						_fds[eventFd]->hangup();
			}
		}
	}
}

