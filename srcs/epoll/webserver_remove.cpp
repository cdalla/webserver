#include "webserver.hpp"
#include "client.hpp"

/*
	REMOVE FD FROM EPOLL INSTANCE
	REMOVE FROM FDS MAP AND DELETE HIS DATA
*/
// void    Webserver::removeFd(int fd, int type, int del)
// {
// 	if (type == CONN)
// 	{
// 		if (epoll_ctl(_epollConn, EPOLL_CTL_DEL, fd, NULL) == -1)
// 			throw WebservException("Failed to remove socket from epoll: " + std::string(strerror(errno)));
// 		if (dynamic_cast<Client *>(_fds[fd]))
// 			delete (_fds[fd]);
// 	}
// 	else if (type == FILES)
// 	{
// 		if (epoll_ctl(_epollFile, EPOLL_CTL_DEL, fd, NULL) == -1)
// 			throw WebservException("Failed to remove socket from epoll: " + std::string(strerror(errno)));
// 		if (del)
// 			delete(_fds[fd]);
// 	}
// 	close (fd);
// 	//_fds.erase(fd);
// }



Webserver::~Webserver()
{
	std::cout << "destructor webserver" << std::endl;
	// std::vector<Fd_handler*> to_remove;
	// for (auto &it : _fds)
	// {
	// 	print_error("here");
	// 	if (dynamic_cast<Client *>(it.second) || dynamic_cast<Cgi *>(it.second) || dynamic_cast<File *>(it.second))
	// 		to_remove.push_back(it.second);
	// }
	// for (auto &it : _fds)
	// {
	// 	print_msg("error");
	// 	if (dynamic_cast<Client *>(it.second) || dynamic_cast<Server *>(it.second))
	// 		removeFromEpoll(it.first, CONN);
	// 	else
	// 		removeFromEpoll(it.first, FILES);
	// }
	// for (std::vector<Fd_handler* >::iterator it = to_remove.begin(); it != to_remove.end(); ++it)
	// 	delete (*it);
	//to_remove.erase() 

	// for (std::unordered_map<int, Fd_handler *>::iterator it = _fds.begin(); it != _fds.end(); ++it)
	// {
	// 	std::cout << it->first << std::endl;
	// 	if (dynamic_cast<Client *>(it->second))
	// 		removeFd(it->first, CONN, 1);
	// 	else if (dynamic_cast<Server *>(it->second))
	// 		removeFd(it->first, CONN, 0);
	// 	else if(dynamic_cast<Cgi *>(it->second))
	// 		remove_Cgi_handler(dynamic_cast<Cgi *>(it->second));
	// 	else if(dynamic_cast<File *>(it->second))
	// 		remove_File_handler(dynamic_cast<File *>(it->second));
	// }
    cleanup();
	close(_epollConn);
	close(_epollFile);
    
}

// void Webserver::remove_Cgi_handler(Cgi* to_remove)
// {
//     if (!to_remove) return;
    
//     print_error("Removing cgi fd for timeout");
    
//     // Store FDs locally as they might be modified during cleanup
//     int inFd = to_remove->get_inFd();
//     int outFd = to_remove->get_outFd();
    
//     // Remove from map first to prevent further access
//     if (inFd != -1) _fds.erase(inFd);
//     if (outFd != -1) _fds.erase(outFd); 
    
//     // Then remove from epoll
//     if (outFd != -1)
//     {
//         epoll_ctl(_epollFile, EPOLL_CTL_DEL, outFd, NULL);
//         close(outFd);
//     }
//     if (inFd != -1)
//     {
//         epoll_ctl(_epollFile, EPOLL_CTL_DEL, inFd, NULL);
//         close(inFd);
//     }
    
//     // Finally delete the object
//     delete to_remove;
//     std::cout << "Cgi removed" << std::endl;
// }

// void	Webserver::remove_File_handler(File *to_remove)
// {
// 	print_error("Removing file fd for timeout");
// 	int inFd = to_remove->get_inFd();
// 	int outFd = to_remove->get_outFd();
// 	if (outFd != -1)
// 		removeFd(outFd, FILES, 0);
// 	if (inFd != -1)
// 		removeFd(inFd, FILES, 1);
// }

// void Webserver::check_timeouts()
// {
//     std::vector<Fd_handler*> to_remove; // Store handlers to remove after iteration
    
//     for (std::unordered_map<int, Fd_handler*>::iterator it = _fds.begin(); it != _fds.end(); ++it)
//     {
//         if (dynamic_cast<Client*>(it->second) && reinterpret_cast<Client*>(it->second)->has_timeout())
//         {
//             dynamic_cast<Client*>(it->second)->request.error = 408;
//             change_event(it->first);
//         }
//         else if((it->second)->has_timeout() && dynamic_cast<Cgi*>(it->second))
//         {
//             to_remove.push_back(it->second);
//         }
//         else if((it->second)->has_timeout() && dynamic_cast<File*>(it->second))
//         {
//             dynamic_cast<File*>(it->second)->_client->request.error = 502;
//             dynamic_cast<File*>(it->second)->_client->file_content.clear();
//             dynamic_cast<File*>(it->second)->_client->status.clear();
//             remove_File_handler(dynamic_cast<File*>(it->second));
//         }
//     }
    
//     // Remove CGI handlers after iteration is complete
//     for (size_t i = 0; i < to_remove.size(); i++) {
//         if (Cgi* cgi = dynamic_cast<Cgi*>(to_remove[i])) {
//             remove_Cgi_handler(cgi);
//         }
//     }
// }

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
    std::cout << _handlers.size() << std::endl;
    std::vector<Fd_handler*>::iterator it;
    it = std::find(_handlers.begin(), _handlers.end(), ptr);
	if (it != _handlers.end())
	{
        print_msg("removing handler");
		_handlers.erase(it);
		delete ptr;
	}
}

//remove a single handler
void    Webserver::removeFd(int fd, int type, int del)
{
    //std::cout << "handler " <<  _fds[fd] << std::endl;
    //std::cout << "fds size: " << _fds.size() << std::endl;
    print_error("removing");
    std::cout << "fd: " << fd << " del: " << del <<  std::endl;
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
            std::cout << "have timeout on fd: " << it->first << std::endl;
            
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