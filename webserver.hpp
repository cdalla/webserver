#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

#include "server.hpp"
#include <vector>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#include "server.hpp"
#include "utils.hpp"
#include "event_handler.hpp"

class WEBSERVER_HPP
{
    private:

        std::vector<Server>             servers;
        std::vector<Client>             clients;
        std::map<int, Event_handler*>   fds;
        int                             epollFd;


    public: 
        
        Webserver(std::string config_file);
        ~Webserver();

        void    servers_init();
        void    create_Epoll();
        
        void    run();

}


#endif