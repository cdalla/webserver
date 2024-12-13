#include "webserver.hpp"
#include "client.hpp"

Webserver::~Webserver()
{
    cleanup();
	close(_epollConn);
	close(_epollFile);    
}

//remove fd from epoll
void Webserver::removeFromEpoll(int fd, int type)
{
	if (type == CONN)
	{
		if (epoll_ctl(_epollConn, EPOLL_CTL_DEL, fd, NULL) == -1)
			throw WebservException("Failed to remove socket from epoll: " + std::string(strerror(errno)));
	}
	else if (type == FILES)
	{
		if (epoll_ctl(_epollFile, EPOLL_CTL_DEL, fd, NULL) == -1)
			throw WebservException("Failed to remove socket from epoll: " + std::string(strerror(errno)));
	}
	close (fd);
}

//remove a single handler if still in memory
void	Webserver::removeHandler(Fd_handler *ptr)
{
    std::vector<Fd_handler*>::iterator it;
    it = std::find(_handlers.begin(), _handlers.end(), ptr);
	if (it != _handlers.end())
	{
		_handlers.erase(it);
		delete ptr;
	}
}

//remove a single handler
void    Webserver::removeFd(int fd, int type, int del)
{
    removeFromEpoll(fd, type);
    if (del)
        removeHandler(_fds[fd]);
    _fds.erase(fd);
}

//check for handler timeout clean and send correct response if necessary
void    Webserver::check_timeouts()
{
    std::vector<int> fd_to_remove;
    for (std::unordered_map<int, Fd_handler*>::iterator it = _fds.begin(); it != _fds.end(); ++it)
    {
        //std::cout << "checking timeout of fd: " << it->first << "is: " << it->second->has_timeout() << std::endl;
        if (!dynamic_cast<Server*>(it->second) && it->second->has_timeout())
        {          
            if (dynamic_cast<Client*>(it->second))
            {
                print_error("client timeout");
                dynamic_cast<Client*>(it->second)->request.error = 408;
                change_event(it->first);
            }
            else if (dynamic_cast<File*>(it->second))
            {
                print_error("file timeout");
                fd_to_remove.push_back(it->first);
                dynamic_cast<File*>(it->second)->_client->request.error = 408;
                dynamic_cast<File*>(it->second)->_client->file_content.clear();
                dynamic_cast<File*>(it->second)->_client->status = "OK";
                removeFromEpoll(it->first, FILES);
                removeHandler(it->second);
            }
            else if (dynamic_cast<Cgi*>(it->second))
            {
                fd_to_remove.push_back(it->first);
                print_error("CGI timeout");
                removeFromEpoll(it->first, FILES);
                removeHandler(it->second); 
            }
        }
    }
    for (auto &it: fd_to_remove)
        _fds.erase(it);
}

//remove everything from epoll, delete allocated memory
void    Webserver::cleanup()
{
    for (std::unordered_map<int, Fd_handler*>::iterator it = _fds.begin(); it != _fds.end(); ++it)
    {
        if (dynamic_cast<Client *>(it->second) || dynamic_cast<Server *>(it->second))
            removeFromEpoll(it->first, CONN);
        else
            removeFromEpoll(it->first, FILES);
        removeHandler(it->second);
    }
    _fds.clear();
    _handlers.clear();
}