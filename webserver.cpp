#include "webserver.hpp"

Webserver::Webserver(std::string filename)
{
    //constructor taking file and process configuration
    //fill the vector
    servers_init();
}

//possible to delete it and put in constructor
//loop servers vector to set up sockets
void    Webserver::servers_init()
{
    std::vector<Server>::iterator it = _servers.begin();
	for (; it != _servers.end(); ++it)
	{
		(*it).createSocket();
	}
}

//epoll_wait loop
void	Webserver::run()
{
    struct epoll_event	events_queue[MAX_EVENTS];
	int readyFds;
	while (true)
	{
		readyFds = epoll_wait(_epollFd, events_queue, MAX_EVENTS, 0); //timeout at zero, no waiting for events on fds
		if (readyFds == -1)
		{
			std::cerr << "failed epoll wait" << std::endl;
			//error or exception
		}
		for (int n = 0; n < readyFds; n++)
		{
            int eventFd = events_queue[n].data.fd;
			std::vector<Server>::iterator servIt = _servers.begin();
			for(; servIt < _servers.end(); ++servIt)
			{
				if( eventFd == (*servIt).get_socket())
                    addClient(eventFd, &(*servIt));
			}
			//process connection
			if (events_queue[n].events & EPOLLIN && _fds.find(eventFd) != _fds.end())
			{
				if (_fds[eventFd]->consume(IN))
					change_event(eventFd, _fds[eventFd]->get_event());

			}
			else if (events_queue[n].events & EPOLLOUT && _fds.find(eventFd) != _fds.end())
			{
				if (_fds[eventFd]->consume(OUT))
					removeFd(eventFd); //close conn and remove fd
			}
			else
			{
				//remove fd from epoll uknown event
				removeFd(eventFd);
			}
		}
	}
}