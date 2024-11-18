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
		int						_pipe[2];
        
	public:
		Client*                 _client;

    	File(std::string filename, Webserver* ptr, Client* client);

		virtual bool    		consume(int event_type);
		        virtual bool            input(void);
        virtual bool            output(void);

};

#endif