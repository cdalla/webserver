#include "webserver.hpp"
#include "client.hpp"

/*
	CREATE EPOLL INSTANCE
	ADD ALL SERVER TO EPOLL AND TO MAP CONTAINER

	*from linux 2.6.8 parameter size on epoll_create is ignored because 
	 epoll data struct is dinamically resized
*/
void    Webserver::create_Epoll()
{
    _epollFd = epoll_create(1); 
	if (_epollFd == -1)
	{
		std::cerr << "Failed to create epoll instance" << std::endl;
		//FATAL ERROR
		//close all server sockets
		//error or exception
	}
    std::vector<Server>::iterator it = _servers.begin();
	for (; it != _servers.end(); ++it)
	{
        addFdToPoll((*it).get_socket(), (*it).get_event());
		addFdToMap((*it).get_socket(), &(*it));
    }
	// std::cout << B_MAGENTA << _fds.size() << RST << std::endl;
}

/*
	ADD NEW SOCKET TO EPOLL INSTANCE
	ACCEPT CONNECTION AND ADD IT TO THE FDS MAP
*/
void    Webserver::addClient(int fd, Server *server)
{
    // if (_fds.find(fd) != _fds.end())
    // {
    //     //fd already present
	// 	//does nothing just skip
	// 	std::cout << "fd " << fd << " already present!" << std::endl;
    //     return ;
    // }
	Socket *client = new Client(server);
	client->set_socket(accept(fd, reinterpret_cast<sockaddr*>(&server->_address), &server->_addrLen));
	if (client->get_socket() == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			std::cout << "Cannot accept new connection" << std::endl;
			delete client;
			return;
		}
		std::cerr << "Error accepting connection" << std::endl;
		//fatal
	}
	// std::cout << "accepted new client on fd: " << client->get_socket() << std::endl;
    make_socket_non_blocking(client->get_socket());
	addFdToPoll(client->get_socket(), server->get_event());
	addFdToMap(client->get_socket(), client);
	// std::cout << "added new client with fd " << client->get_socket() << " on server fd: "<< server->get_socket() << std::endl;
}

void	Webserver::addFdToMap(int fd, Socket *socket)
{
	if (_fds.find(fd) != _fds.end())
	{
		//fd already present
		std::cout << "fd " << fd << " already present!" << std::endl;
		return;
	}
	_fds[fd] = socket;
}

/*
	SET EVENT FOR THE FD
	ADD IT TO THE EPOLL INSTANCE
*/
void    Webserver::addFdToPoll(int fd, struct epoll_event *event) 
{

	(*event).events = EPOLLIN;
	(*event).data.fd = fd;
	epoll_event eventboh{};
	eventboh.events = EPOLLIN;
	eventboh.data.fd = fd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &eventboh) == -1) {
		std::cerr << "Failed to add socket to epoll instance" << std::endl;
		//error or exception
		//close socket and epollfd
	}
}

void	Webserver::change_event(int fd, struct epoll_event *event)
{
	epoll_event eventboh{};
	eventboh.events = EPOLLOUT;
	eventboh.data.fd = fd;
	(*event).events = EPOLLOUT;
	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &eventboh) == -1) {
		std::cerr << "Failed to modify socket event in epoll instance" << std::endl;
		//FATAL ERROR
		//error or exception
		//close socket and epollfd
	}
	// std::cout << "succefully changed event on fd: " << fd << std::endl;
}

/*
	REMOVE FD FROM EPOLL INSTANCE
	REMOVE FROM FDS MAP AND DELETE HIS DATA
*/
void    Webserver::removeFd(int fd)
{
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) == -1) {
		std::cerr << "Failed to delete socket event in epoll instance" << std::endl;
		//FATAL ERROR
		std::cerr << strerror(errno) << std::endl;
		exit(1);
	}
	if (dynamic_cast<Client *>(_fds[fd]))
		delete (_fds[fd]);
	_fds.erase(fd);
	close (fd);
}