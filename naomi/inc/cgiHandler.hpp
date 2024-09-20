#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

#include <string>
#include <map>
#include <vector>

class Request;
class Response;

class cgiHandler {
	
	public:
		cgiHandler(Request &request, Response &response, char **envp);
		~cgiHandler(void);

		// std::string	getBody(void);
		// char		**getEnv(void);
	
		void	init(void);
		void	init_env(Request &request, char **envp);
		void	execute(void);

		int		request_pipe[2];
		int		response_pipe[2];
		pid_t	pid;
		int		pid_status;
		int		old_stdin;
		int		old_stdout;
		int		bodyLength;
		std::string rawResponse;
		char		buf[1024];
	
	private:
		std::string	_body;
		std::vector<char>	_bodyVector;
		char		**_env;
};

// std::ostream&   operator<<(std::ostream& out, cgiHandler const &obj);

#endif