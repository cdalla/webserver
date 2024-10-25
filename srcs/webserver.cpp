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
	std::map<int, Socket *>::iterator it = _fds.begin();
	for (; it != _fds.end(); ++it)
	{
		removeFd(it->first);
	}
}

/*
	LOOP TO INITIALIZE SERVER SOCKET
*/
void    Webserver::servers_init()
{
	for (std::vector<VirtualServer>::iterator it = config.servers.begin(); it != config.servers.end(); it++) {
		_servers.push_back(Server(*it));
	}
    std::vector<Server>::iterator it = _servers.begin();
	for (; it != _servers.end(); ++it)
	{
		(*it).createSocket();
	}
}

void	Webserver::check_timeouts()
{
	std::map<int, Socket *>::iterator it = _fds.begin();
	for (; it != _fds.end(); ++it)
	{
		if (dynamic_cast<Client *>(it->second) && reinterpret_cast<Client *>(it->second)->has_timeout())
		{
			print_error("Removing fd for timeout");
			removeFd(it->first);
		}
	}
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
		readyFds = epoll_wait(_epollFd, events_queue, MAX_EVENTS, 10);
		if (readyFds == -1)
			throw WebservException("Epoll wait failure: " + std::string(strerror(errno)));
		
		for (int n = 0; n < readyFds; n++)
		{
            int eventFd = events_queue[n].data.fd;
			std::vector<Server>::iterator servIt = _servers.begin();
			for (; servIt < _servers.end(); servIt++) {
				if( eventFd == (*servIt).get_socket()) {
                    addClient(eventFd, &(*servIt));
				}
			}
				if (events_queue[n].events & EPOLLIN && _fds.find(eventFd) != _fds.end()) {
					if (_fds[eventFd]->consume(IN))
						change_event(eventFd);
				}
				else if (events_queue[n].events & EPOLLOUT && _fds.find(eventFd) != _fds.end()) {
					if (_fds[eventFd]->consume(OUT))
						removeFd(eventFd);
				}
				else
					removeFd(eventFd);
		}
	}
}