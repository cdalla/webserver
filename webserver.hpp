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

# include "client.hpp"
# include "server.hpp"
//# include "utils.hpp"
# include "event_handler.hpp"

#define MAX_EVENTS 10
#define IN 0
#define OUT 1

class Webserver
{
    private:

        std::vector<Server>             _servers;
        std::vector<Client>             _clients;
        std::map<int, Event_handler*>   _fds;
        int                             _epollFd;


        void    servers_init();
        void    create_Epoll();
        void	addFdToMap(int fd, Server *server);
        void    addFdToPoll(int fd, struct epoll_event *event);
        void    addClient(int fd, Server *server);


    public: 
        
        Webserver(std::string config_file);
        ~Webserver();

        
        void    run();

};


void make_socket_non_blocking(int socket_fd);

#endif