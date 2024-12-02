#include "webserver.hpp"
#include "client.hpp"



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
			removeFd(it->first, CONN, 1);
		else if((it->second)->has_timeout() && dynamic_cast<Cgi *>(it->second))
			remove_Cgi_handler(dynamic_cast<Cgi *>(it->second));
		else if((it->second)->has_timeout() && dynamic_cast<File *>(it->second))
			remove_File_handler(dynamic_cast<File *>(it->second));
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

void	Webserver::remove_Cgi_handler(Cgi *to_remove)
{
	print_error("Removing fd for timeout");
	int inFd = to_remove->get_inFd();
	int outFd = to_remove->get_outFd();
	if (inFd != -1)
		removeFd(inFd, FILES, 0);
	if (outFd != -1)
		removeFd(outFd, FILES, 1);
}

void	Webserver::remove_File_handler(File *to_remove)
{
	print_error("Removing fd for timeout");
	int inFd = to_remove->get_inFd();
	int outFd = to_remove->get_outFd();
	if (inFd != -1)
		removeFd(inFd, FILES, 0);
	if (outFd != -1)
		removeFd(outFd, FILES, 1);
}

void	Webserver::check_timeouts()
{
	std::map<int, Fd_handler *>::iterator it = _fds.begin();
	for (; it != _fds.end(); ++it)
	{
		if (dynamic_cast<Client *>(it->second) && reinterpret_cast<Client *>(it->second)->has_timeout())
		{
			print_error("Removing fd for timeout");
			removeFd(it->first, CONN, 0);
		}
		else if((it->second)->has_timeout() && dynamic_cast<Cgi *>(it->second))
			remove_Cgi_handler(dynamic_cast<Cgi *>(it->second));
		else if((it->second)->has_timeout() && dynamic_cast<File *>(it->second))
			remove_File_handler(dynamic_cast<File *>(it->second));
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
		//check_timeouts();
		{
			readyFds = epoll_wait(_epollConn, events_queue, MAX_EVENTS, 10);
			if (readyFds == -1)
				throw WebservException("Epoll wait failure: " + std::string(strerror(errno)));
			for (int n = 0; n < readyFds; n++)
			{
            	int eventFd = events_queue[n].data.fd;
				//std::cout << "event fd = " << eventFd << std::endl;
				if (events_queue[n].events & EPOLLIN && _fds.find(eventFd) != _fds.end())
					_fds[eventFd]->input();
				else if (events_queue[n].events & EPOLLOUT && _fds.find(eventFd) != _fds.end())
					_fds[eventFd]->output();
				else
					removeFd(eventFd, CONN, 0);
			}
		}
		{
			readyFds = epoll_wait(_epollFile, events_queue, MAX_EVENTS, 10);
			if (readyFds == -1)
				throw WebservException("Epoll wait failure: " + std::string(strerror(errno)));
			for (int n = 0; n < readyFds; n++)
			{
            	int eventFd = events_queue[n].data.fd;
				//std::cout << "event fd = " << eventFd << std::endl;
				if (events_queue[n].events & EPOLLIN && _fds.find(eventFd) != _fds.end())
					_fds[eventFd]->input();
				else if (events_queue[n].events & EPOLLOUT && _fds.find(eventFd) != _fds.end()) 
					_fds[eventFd]->output();
				else if (events_queue[n].events & EPOLLHUP && _fds.find(eventFd) != _fds.end())
						_fds[eventFd]->hangup();
				else
					removeFd(eventFd, FILES, 1);
			}
		}
	}
}

