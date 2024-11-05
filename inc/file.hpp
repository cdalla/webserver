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
        Client*                 _client;

	public:
	
    	File(int fd, Webserver* ptr);
		~File() = default;

		virtual bool    		consume(int event_type);

};

#endif