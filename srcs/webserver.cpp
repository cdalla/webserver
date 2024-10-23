#include "webserver.hpp"
# include "client.hpp"


Webserver::Webserver(const char *default_config) : config(default_config)
{
    //constructor taking file and process configuration
    //fill the vector

	config.parseConfig();
	servers_init();
	create_Epoll();
}

Webserver::~Webserver()
{
	clean();
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
	// std::cout << B_MAGENTA << _fds.size() << RST << std::endl;
}

/*
	delete all fds from epoll
	delete all clients
	close all fds
*/
void	Webserver::clean()
{
	std::map<int, Socket *>::iterator it = _fds.begin();
	for (; it != _fds.end(); ++it)
	{
		removeFd(it->first);
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
	//return ; //! ADDED THIS FOR EASY CONFIG CHECKING OBV THIS NEEDS TO GO!!!
	while (true)
	{
		readyFds = epoll_wait(_epollFd, events_queue, MAX_EVENTS, 10);
		if (readyFds == -1)
		{
			std::cerr << "failed epoll wait" << std::endl;
			//FATAL ERROR
			exit(1);
			//error or exception
		}
		// std::cout << "Waiting on events.." << std::endl;
		for (int n = 0; n < readyFds; n++)
		{
            int eventFd = events_queue[n].data.fd;
			// std::cout << "event on fd: " << eventFd << std::endl;
			bool newClient = false;
			std::vector<Server>::iterator servIt = _servers.begin();
			for (; servIt < _servers.end(); servIt++) {
				if( eventFd == (*servIt).get_socket()) {
                    addClient(eventFd, &(*servIt));
					newClient = true;
				}
			}
			if (!newClient) {
				if (events_queue[n].events & EPOLLIN && _fds.find(eventFd) != _fds.end()) {
					if (_fds[eventFd]->consume(IN))
						change_event(eventFd, ((Client *) _fds[eventFd])->_server->get_event());
				}
				else if (events_queue[n].events & EPOLLOUT && _fds.find(eventFd) != _fds.end()) {
					if (_fds[eventFd]->consume(OUT)) {
						// std::cout << "removing fd " << eventFd << " after successful return of consume function" << std::endl;
						removeFd(eventFd);
					}
					else {
						std::cout << "removing fd " << eventFd << " after error in consume function" << std::endl;
						removeFd(eventFd);
					}
				}
				else {
					//remove fd from epoll uknown event
					std::cout << "removing fd " << eventFd << " from epoll because it is an unknown event" << std::endl;
					removeFd(eventFd);
				}
			}
		}
	}
}