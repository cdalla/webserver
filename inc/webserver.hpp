#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# include <vector>
# include <map>
# include <sys/socket.h>
# include <sys/epoll.h>
# include <arpa/inet.h>
# include <fcntl.h>
# include <iostream>
# include <unistd.h>
# include <cstring>

# include "colours.hpp"
# include "structs.hpp"
# include "config.hpp"
//# include "server.hpp"
// # include "client.hpp"
# include "fd_handler.hpp"
// # include "response.hpp"
// # include "request.hpp"
# include "utils.hpp"
# include "WebservException.hpp"

//# define MAX_EVENTS 100
# define IN 0
# define OUT 1
# define CONN 2
# define FILES 3
# define MAX_SIZE 1024
# define MAX_BUFF 10000
//# define MAX_CONNECTIONS 10

class Server;

class Webserver
{
    
    private:

        std::vector<Server>             _servers;
        std::map<int, Fd_handler*>      _fds;
        int                             _epollConn;
        int                             _epollFile;

        void    servers_init();
        void    create_Epoll();
        void    check_timeouts();

    public: 
        
        Webserver(std::string default_config);
        ~Webserver();

        void    addFdToPoll(int fd, int epollFd, uint32_t events);
        void    addFdToMap(int fd, Fd_handler *ptr);
        void    change_event(int fd);
        void    removeFd(int fd, int type, int del);
        bool	is_in_map(int fd);
        int     get_EpollFd(int type);
        
        class Config		config;
        void    run();

};

#endif