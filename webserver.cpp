#include "webserver.hpp"

Webserver::Webserver(std::string filename)
{
    //constructor taking file and process configuration
    //fill the vector
	
	{
		//FOR TEST
		(void) filename;
   		Server test;
		test.set_socket(8080);
		_servers.push_back(test);
	}

	servers_init();
	create_Epoll();

}

/*
	LOOP TO INITIALIZE SERVER SOCKET
*/
void    Webserver::servers_init()
{
    std::vector<Server>::iterator it = _servers.begin();
	for (; it != _servers.end(); ++it)
	{
		(*it).createSocket();
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
	int readyFds;
	while (true)
	{
		readyFds = epoll_wait(_epollFd, events_queue, MAX_EVENTS, 0);
		if (readyFds == -1)
		{
			std::cerr << "failed epoll wait" << std::endl;
			exit(1);
			//error or exception
		}
		std::cout << "Waiting on events.." << std::endl; 
		for (int n = 0; n < readyFds; n++)
		{
            int eventFd = events_queue[n].data.fd;
			std::vector<Server>::iterator servIt = _servers.begin();
			for(; servIt < _servers.end(); ++servIt)
			{
				if( eventFd == (*servIt).get_socket())
                    addClient(eventFd, &(*servIt));
			}
			if (events_queue[n].events & EPOLLIN && _fds.find(eventFd) != _fds.end())
			{
				if (_fds[eventFd]->consume(IN))
					change_event(eventFd, _fds[eventFd]->get_event());

			}
			else if (events_queue[n].events & EPOLLOUT && _fds.find(eventFd) != _fds.end())
			{
				if (_fds[eventFd]->consume(OUT))
					removeFd(eventFd);
			}
			else
			{
				//remove fd from epoll uknown event
				removeFd(eventFd);
			}
		}
	}
}