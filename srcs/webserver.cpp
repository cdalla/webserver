#include "webserver.hpp"
# include "client.hpp"


Webserver::Webserver(std::string default_config) : config(default_config.c_str())
{
	config.parseConfig();
	servers_init();
	create_Epoll();
}

Webserver::~Webserver()
{
	std::map<int, Fd_handler *>::iterator it = _fds.begin();
	for (; it != _fds.end(); ++it)
	{
		if (dynamic_cast<Client *>(it->second) || dynamic_cast<Server *>(it->second))
			removeFd(it->first, CONN);
		else
			removeFd(it->first, FILES);
	}
}

/*
	LOOP TO INITIALIZE SERVER SOCKET
*/
void    Webserver::servers_init()
{
	for (std::vector<VirtualServer>::iterator it = config.servers.begin(); it != config.servers.end(); it++) {
		_servers.push_back(Server(*it, this));
	}
    std::vector<Server>::iterator it = _servers.begin();
	for (; it != _servers.end(); ++it)
	{
		(*it).createSocket();
	}
}

void	Webserver::check_timeouts()
{
	std::map<int, Fd_handler *>::iterator it = _fds.begin();
	for (; it != _fds.end(); ++it)
	{
		if (dynamic_cast<Client *>(it->second) && reinterpret_cast<Client *>(it->second)->has_timeout())
		{
			print_error("Removing fd for timeout");
			removeFd(it->first, CONN);
		}
		// else if((it->second)->has_timeout() && (dynamic_cast<Cgi *>(it->second) || dynamic_cast<File *>(it->second)))
		// {
		// 	print_error("Removing fd for timeout");
		// 	removeFd(it->first, FILES)
		// }
	}
}


bool	Webserver::is_in_map(int fd)
{
	if (_fds.find(fd) != _fds.end())
		return true;
	return false;	
}

void    Webserver::addFdToMap(int fd, Fd_handler *ptr)
{
	_fds[fd] = ptr;
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
				if (events_queue[n].events & EPOLLIN && _fds.find(eventFd) != _fds.end()) {
					if (_fds[eventFd]->consume(IN))
						change_event(eventFd);
				}
				else if (events_queue[n].events & EPOLLOUT && _fds.find(eventFd) != _fds.end()) {
					if (_fds[eventFd]->consume(OUT))
						removeFd(eventFd, CONN);
				}
				else
					removeFd(eventFd, CONN);
			}
		}
		{
			readyFds = epoll_wait(_epollFile, events_queue, MAX_EVENTS, 10);
			if (readyFds == -1)
				throw WebservException("Epoll wait failure: " + std::string(strerror(errno)));
			for (int n = 0; n < readyFds; n++)
			{
            	int eventFd = events_queue[n].data.fd;
				if (events_queue[n].events & EPOLLIN && _fds.find(eventFd) != _fds.end()) {
					_fds[eventFd]->consume(IN);
				}
				else if (events_queue[n].events & EPOLLOUT && _fds.find(eventFd) != _fds.end()) {
					if (_fds[eventFd]->consume(OUT))
						removeFd(eventFd, FILES);
				}
				else
					removeFd(eventFd, FILES);
			}
		}
	}
}