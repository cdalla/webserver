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
# include "server.hpp"
// # include "client.hpp"
# include "socket.hpp"
// # include "response.hpp"
// # include "request.hpp"
# include "utils.hpp"
# include "WebservException.hpp"

//# define MAX_EVENTS 100
# define IN 0
# define OUT 1
# define MAX_SIZE 1024
//# define MAX_CONNECTIONS 10

class Server;

class Webserver
{
    
    private:

        std::vector<Server>             _servers;
        std::map<int, Socket*>          _fds;
        int                             _epollFd;

        void    servers_init();
        void    create_Epoll();
        void    addFdToPoll(int fd);
        void    addClient(int fd, Server *server);
        void    removeFd(int fd);
        void    change_event(int fd);
        void    check_timeouts();

    public: 
        
        Webserver(std::string default_config);
        ~Webserver();

        class Config		config;
        void    run();

};

#endif