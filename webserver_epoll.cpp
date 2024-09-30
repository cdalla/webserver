#include "webserver.hpp"

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

void    Webserver::addClient(int fd, Server *server)
{
    if (_fds.find(fd) != _fds.end())
    {
        //fd already present 
        return ;
    }
	Socket *client = new Client(server);
    //add new socket to epoll instance
	//accept connection and add it to connections vector
	client->set_socket(accept(fd, (struct sockaddr*)client->get_address(), client->get_addLen()));
	if (client->get_socket() == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			//busy server cant accept connection now
			return;
		}
		std::cerr << "Error accepting connection" << std::endl;				
	}
    make_socket_non_blocking(client->get_socket());
	addFdToPoll(client->get_socket(), client->get_event());
	addFdToMap(client->get_socket(), client);
}

void	Webserver::addFdToMap(int fd, Socket *socket)
{
	if (_fds.find(fd) != _fds.end())
	{
		//fd already present
		return;
	}
	_fds[fd] = socket;
}

//add fds to epoll pool
void    Webserver::addFdToPoll(int fd, struct epoll_event *event) 
{

		(*event).events = EPOLLIN | EPOLLERR | EPOLLHUP;
		(*event).data.fd = fd;
		if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, event) == -1)
		{
			std::cerr << "Failed to add socket to epoll instance" << std::endl;
			//error or exception
			//close socket and epollfd
		}
}

void	Webserver::change_event(int fd, struct epoll_event *event)
{
	(*event).events = EPOLLOUT;
	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, event) == -1)
	{
		std::cerr << "Failed to modify socket event in epoll instance" << std::endl;
		//error or exception
		//close socket and epollfd
	}
}

//remove fd from Epoll, map
void    Webserver::removeFd(int fd)
{
    //remove from Epoll
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL);
	//remove from _fds
	delete (_fds[fd]);
	_fds.erase(fd);
	close (fd);
}