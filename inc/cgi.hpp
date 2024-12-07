#ifndef CGI_HPP
# define CGI_HPP

#include "webserver.hpp"
#include "client.hpp"
// #include "server.hpp"
#include "fd_handler.hpp"
#include "structs.hpp"

#include <sys/wait.h>

class Webserver;
class Client;

class Cgi : public Fd_handler
{
	private:    
    
		Webserver*				_main;
        Client*                 _client;
		int						_pipeIn[2];
		int						_pipeOut[2];
		int						_outFd;
		int						_inFd;
		const char*				_script;
		char* const*			_env;
		std::string				_body;
		pid_t					_pid;
		size_t					_pos;
		bool					_writeFinished;

		void					execute_child();
		
	public:
	
    	Cgi(Webserver* ptr, const char *script, char * const *env, std::string body, Client *client);
		~Cgi();

		virtual void            input(void);
        virtual void            output(void);
		virtual void			hangup(void);
		int 					get_inFd();
		int 					get_outFd();

};

#endif