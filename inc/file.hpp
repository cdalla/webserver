#ifndef FILE_HPP
# define FILE_HPP

#include "webserver.hpp"
#include "client.hpp"
// #include "server.hpp"
#include "fd_handler.hpp"
#include "structs.hpp"

class Webserver;
class Client;

class File : public Fd_handler
{
	private:    
    
		Webserver*				_main;
		std::string 			_content;
        
	public:
		Client*                 _client;

    	File(int fd, Webserver* ptr, Client* client);

		virtual bool    		consume(int event_type);

};

#endif