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
	std::cout << "destructor" << std::endl;
	std::map<int, Fd_handler *>::iterator it = _fds.begin();
	for (; it != _fds.end(); ++it)
	{
		if (dynamic_cast<Client *>(it->second) || dynamic_cast<Server *>(it->second))
			removeFd(it->first, CONN, 0);
		else if(dynamic_cast<Cgi *>(it->second))
			remove_Cgi_handler(dynamic_cast<Cgi *>(it->second));
		else if(dynamic_cast<File *>(it->second))
			remove_File_handler(dynamic_cast<File *>(it->second));
	}
	close(_epollConn);
	close(_epollFile);
}

bool Webserver::is_PortInUse(unsigned int port, std::string ip, std::vector<Server>::iterator end)
{
	std::vector<Server>::iterator it = _servers.begin();
	for (; it != end; ++it)
	{
		if ((*it).getPort() == port)
			return true;
	}
	return false;
}

/*
	LOOP TO INITIALIZE SERVER SOCKET
*/
void    Webserver::servers_init()
{
	for (std::vector<VirtualServer>::iterator it = config.servers.begin(); it != config.servers.end(); it++) {
		
		_servers.push_back(Server(*it, this, &config));
	}
    std::vector<Server>::iterator it = _servers.begin();
	for (; it != _servers.end(); ++it)
	{
		if (!is_PortInUse((*it).getPort(),(*it).getIp(), it))
			(*it).createSocket();
	}
}

void Webserver::remove_Cgi_handler(Cgi* to_remove)
{
    if (!to_remove) return;
    
    print_error("Removing cgi fd for timeout");
    
    // Store FDs locally as they might be modified during cleanup
    int inFd = to_remove->get_inFd();
    int outFd = to_remove->get_outFd();
    
    // Remove from map first to prevent further access
    if (inFd != -1) _fds.erase(inFd);
    if (outFd != -1) _fds.erase(outFd);
    
    // Then remove from epoll
    if (outFd != -1)
    {
        epoll_ctl(_epollFile, EPOLL_CTL_DEL, outFd, NULL);
        close(outFd);
    }
    if (inFd != -1)
    {
        epoll_ctl(_epollFile, EPOLL_CTL_DEL, inFd, NULL);
        close(inFd);
    }
    
    // Finally delete the object
    delete to_remove;
    std::cout << "Cgi removed" << std::endl;
}

void	Webserver::remove_File_handler(File *to_remove)
{
	print_error("Removing file fd for timeout");
	int inFd = to_remove->get_inFd();
	int outFd = to_remove->get_outFd();
	if (outFd != -1)
		removeFd(outFd, FILES, 0);
	if (inFd != -1)
		removeFd(inFd, FILES, 1);
}

void Webserver::check_timeouts()
{
    std::vector<Fd_handler*> to_remove; // Store handlers to remove after iteration
    
    for (std::map<int, Fd_handler*>::iterator it = _fds.begin(); it != _fds.end(); ++it)
    {
        if (dynamic_cast<Client*>(it->second) && reinterpret_cast<Client*>(it->second)->has_timeout())
        {
            dynamic_cast<Client*>(it->second)->request.error = 408;
            change_event(it->first);
        }
        else if((it->second)->has_timeout() && dynamic_cast<Cgi*>(it->second))
        {
            to_remove.push_back(it->second);
        }
        else if((it->second)->has_timeout() && dynamic_cast<File*>(it->second))
        {
            dynamic_cast<File*>(it->second)->_client->request.error = 502;
            dynamic_cast<File*>(it->second)->_client->file_content.clear();
            dynamic_cast<File*>(it->second)->_client->status.clear();
            remove_File_handler(dynamic_cast<File*>(it->second));
        }
    }
    
    // Remove CGI handlers after iteration is complete
    for (size_t i = 0; i < to_remove.size(); i++) {
        if (Cgi* cgi = dynamic_cast<Cgi*>(to_remove[i])) {
            remove_Cgi_handler(cgi);
        }
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
	//throw WebservException("testing close fd");
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
				if (events_queue[n].events & EPOLLIN && _fds.find(eventFd) != _fds.end())
					_fds[eventFd]->input();
				else if (events_queue[n].events & EPOLLOUT && _fds.find(eventFd) != _fds.end()) 
					_fds[eventFd]->output();
				else if (events_queue[n].events & EPOLLHUP && _fds.find(eventFd) != _fds.end())
						_fds[eventFd]->hangup();
			}
		}
	}
}

