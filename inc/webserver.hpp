#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# include <vector>
# include <map>
# include <unordered_map>
# include <algorithm>
# include <sys/socket.h>
# include <sys/epoll.h>
# include <arpa/inet.h>
# include <fcntl.h>
# include <iostream>
# include <unistd.h>
# include <cstring>

# include "defines.hpp"
# include "colours.hpp"
# include "structs.hpp"
# include "config.hpp"
# include "utils.hpp"
# include "WebservException.hpp"
# include "fd_handler.hpp"
# include "file.hpp"
# include "cgi.hpp"

class Server;
class File;
class Cgi;

class Webserver
{
    
    private:

        std::vector<Server>                     _servers;
        std::vector<Fd_handler*>                _handlers;
        std::unordered_map<int, Fd_handler*>    _fds;
        int                                     _epollConn;
        int                                     _epollFile;

        void    servers_init();
        void    create_Epoll();
		bool 	is_PortInUse(unsigned int port, std::string ip, std::vector<Server>::iterator end);
        void	removeHandler(Fd_handler *ptr);
        void	addHandler(Fd_handler *ptr);
        void    removeFromEpoll(int fd, int type);
        void    check_timeouts();
        void    cleanup();
        void    run();

    public: 
        
        class Config		config;
        Webserver(std::string default_config);
        ~Webserver();

        void    addFdToPoll(int fd, int epollFd, uint32_t events);
        void    addFdToMap(int fd, Fd_handler *ptr);
        void    change_event(int fd);
        bool	is_in_map(int fd);
        int     get_EpollFd(int type);
        void    removeFd(int fd, int type, int del);
        

};

#endif