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
#include "file.hpp"
#include "cgi.hpp"
#include "defines.hpp"

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
        //void    check_timeouts();

    public: 
        
        Webserver(std::string default_config);
        ~Webserver();

        void    addFdToPoll(int fd, int epollFd, uint32_t events);
        void    addFdToMap(int fd, Fd_handler *ptr);
        void    change_event(int fd);
       // void    removeFd(int fd, int type, int del);
        //void	remove_Cgi_handler(Cgi *to_remove);
        //void	remove_File_handler(File *to_remove);
        bool	is_in_map(int fd);
		bool 	is_PortInUse(unsigned int port, std::string ip, std::vector<Server>::iterator end);
        int     get_EpollFd(int type);

        void	removeHandler(Fd_handler *ptr);
        void	addHandler(Fd_handler *ptr);
        void    removeFromEpoll(int fd, int type);
        void    removeFd(int fd, int type, int del);
        void    check_timeouts();
        void    cleanup();


        
        class Config		config;
        void    run();

};

#endif