#include "webserver.hpp"

Webserver::Webserver(std::string filename)
{
    //constructor taking file and process configuration
    //fill the vector
    servers_init();
}

Webserver::~Webserver(){;}

//loop servers vector to set up sockets
void    Webserver::servers_init()
{
    std::vector<Server>::iterator it = _servers.begin();
	for (; it != _servers.end(); ++it)
	{
		(*it).createSocket();
	}
}

void    Webserver::create_Epoll()
{
    _epollFd = epoll_create(1); //from linux 2.6.8 parameter size is ignored because epoll data struct is dinamically resized
	if (_epollFd == -1)
	{
		std::cerr << "Failed to create epoll instance" << std::endl;
		//close all server sockets
		//error or exception
	}
    std::vector<Server>::iterator it = _servers.begin();
	for (; it != _servers.end(); ++it)
	{
        addFdToPoll((*it).get_socket(), (*it).get_event());
		addFdToMap((*it).get_socket(), &(*it));
    }
}

void	Webserver::addFdToMap(int fd, Server *server)
{
	//create handler and add to map
	if (_fds.find(fd) != _fds.end())
	{
		//fd already present
		return;
	}
	Event_handler *handler = new Event_handler(server);
	_fds[fd] = handler;
}

//add fds to epoll pool
void    Webserver::addFdToPoll(int fd, struct epoll_event *event) 
{

		(*event).events = EPOLLIN | EPOLLERR | EPOLLHUP;
		(*event).data.fd = fd;
		if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, event) == -1)
		{
			std::cerr << "Failed to add server socket to epoll instance" << std::endl;
			//error or exception
			//close socket and epollfd
		}
}

void    Webserver::addClient(int fd, Server *server)
{

	Client client;

    //add new socket to epoll instance
	//accept connection and add it to connections vector
	client.set_socket(accept((server->get_socket()), (struct sockaddr*)client.get_address(), client.get_addLen()));
	if (client.get_socket() == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			//busy server cant accept connection now
			return;
		}
		std::cerr << "Error accepting connection" << std::endl;				
	}
    make_socket_non_blocking(client.get_socket());
	addFdToPoll(client.get_socket(), client.get_event());
	addFdToMap(client.get_socket(), server);
	_clients.push_back(client);
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
				// maybe first loop to check new connection and then event handler if
				else
				{
					//process connections
					if (events_queue[n].events & EPOLLIN && _fds.find(eventFd) != _fds.end())
						_fds[eventFd]->consume(IN);
					else if (events_queue[n].events & EPOLLOUT)
						_fds[eventFd]->consume(OUT);
					//else if (cgi)
					//else
						//remove fd from epoll uknown event
				}
			}
		}
	}
}