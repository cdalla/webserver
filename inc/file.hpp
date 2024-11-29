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
		char					_buff[MAX_BUFF];
		ssize_t					_file_size;
		int						_pipe[2];
        
	public:
		Client*                 _client;

    	File(std::string filename, Webserver* ptr, Client* client);

		virtual void          	input(void);
        virtual void            output(void);
		virtual void			hangup(void){return;}

};

#endif