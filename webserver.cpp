#include "Webserver.hpp"

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
    std::vector<Servers>::iterator it = servers.begin();
	for (; it != servers.end(); ++it)
	{
		(*it).createSocket();
	}
}

void    Webserver::create_Epoll()
{
    epollFd = epoll_create(1); //from linux 2.6.8 parameter size is ignored because epoll data struct is dinamically resized
	if (epollFd == -1)
	{
		std::cerr << "Failed to create epoll instance" << std::endl;
		//close all server sockets
		//error or exception
	}
    std::vector<Server>::iterator it = servers.begin();
	for (; it != servers.end(); ++it)
	{
        addFdToPoll((*it).socket, (*it).event);
		addFdToMap((*it).socket, (*it));
    }
}

void	addFdToMap(int fd, Server *server)
{
	//create handler and add to map
	if (fds.find(fd) != fds.end())
	{
		//fd already present
		return;
	}
	Event_handler *handler = new Event_handler(server);
	fds[fd] = handler;
}

//add fds to epoll pool
void    Webserver::addFdToPoll(int fd, struct epoll_event *event) 
{

		event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
		event.data.fd = fd;
		if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
		{
			std::cerr << "Failed to add server socket to epoll instance" << std::endl;
			//error or exception
			//close socket and epollfd
		}
}

void    Webserver::addClient(int fd, Server &server)
{

	Client client = new Client();

    //add new socket to epoll instance
	//accept connection and add it to connections vector
	client.addrLen = (socklen_t *)sizeof(new_conn.addr); //client address
	client.socket = accept(((*servIt).socket), (struct sockaddr*)&new_conn.addr, new_conn.addrLen);
	if (client.socket == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			//busy server cant accept connection now
			return;
		}
		std::cerr << "Error accepting connection" << std::endl;				
	}
    make_socket_non_blocking(client.socket);
	addFdToPoll(client.socket, client.event);
	addFdToMap(client.socket, server);
	clients.push_back(client);
}

//epoll_wait loop
void Webserver::run()
{
    struct epoll_event	events_queue[MAX_EVENTS];
	int readyFds;
	while (true)
	{
		readyFds = epoll_wait(epollFd, events_queue, MAX_EVENTS, 0); //timeout at zero, no waiting for events on fds
		if (readyFds == -1)
		{
			std::cerr << "failed epoll wait" << std::endl;
			//error or exception
		}
		for (int n = 0; n < readyFds; n++)
		{
            int eventFd = events_queue[n].data.fd;
			std::vector<Server>::iterator servIt = servers.begin();
			for(; servIt < servers.end(); ++servIt)
			{
				if( eventFd == (*servIt).socket)
                    addClient(eventFd, *servIt);
				else
				{
					//process connections
					if (events_queue[n].events & EPOLLIN)
						//read data
					else if (events_queue[n].events & EPOLLOUT)
						//send data
					//else if (cgi)
					else
						//remove fd from epoll uknown event
				}
			}
		}
	}
}