#include <unistd.h>
#include "cgiHandler.hpp"
#include "request.hpp"
#include "response.hpp"
#include <iostream>
#include "colours.hpp"

cgiHandler::cgiHandler(Request &request, Response &response, char **envp) : pid_status(0) {

	init_env(request, envp);
	_body.resize(bodyLength);
	_body = request.getBody();
}

cgiHandler::~cgiHandler(void) {}

void	cgiHandler::init(void) {

	if (pipe(request_pipe) == -1 || pipe(response_pipe) == -1)
	{
		perror("Failure opening pipes");
		exit(1); //! chaaaaange
	}
	old_stdin = dup(STDIN_FILENO);
	old_stdout = dup(STDOUT_FILENO);
	dup2(request_pipe[0], STDIN_FILENO);
	dup2(response_pipe[1], STDOUT_FILENO);
}

void	cgiHandler::execute(void) {
	
	int		valRead;
	char	*argv[] = {
		// const_cast<char *>("/Users/naomisterk/codam/webbieservie/www/cgi-bin/save_file.py"),
		const_cast<char *>("/usr/bin/python3"),
		const_cast<char *>("/Users/naomisterk/codam/webbieservie/www/cgi-bin/save_file.py"),
		nullptr
	};

	init();
	pid = fork(); //! implement actual error handling if fork failure
	if (pid < 0) {
		perror("forking process failed");
		exit(1);
	}
	// execute CGI. If execve fails, send 'Error' to stdout and exit child process.
	if (pid == 0) { //! Need to check later that all pipes are closed correctly
		close(request_pipe[0]);
		write(request_pipe[1], _body.c_str(), bodyLength);
		close(request_pipe[1]);
		if (execve("/usr/bin/python3", argv, _env) < 0)
		{
			perror("motherfucking execve: ");
			exit(1);
		}
	}
	close(request_pipe[0]); //! If problems with cgi - check if not closing request pipe too early
	close(request_pipe[1]); //! If problems with cgi - check if not closing request pipe too early
	waitpid(pid, &pid_status, 0); // wait for child process
	valRead = read(response_pipe[0], buf, 1023);
	if (valRead < 0)
	{
		perror("problem reading from response pipe");
		close(response_pipe[0]);
		close(response_pipe[1]);
	}
	buf[valRead] = '\0';
	close(response_pipe[0]);
	close(response_pipe[1]);
	rawResponse.append(buf);
	dup2(old_stdin, STDIN_FILENO);
	dup2(old_stdout, STDOUT_FILENO);
}

//! Make this whole function C++ style
//! add malloc protection
//! leaks and stuff
void	cgiHandler::init_env(Request &request, char **envp) {

	size_t	len = 0;
	std::string	tmp;
	std::map<std::string, std::string> headers = request.getHeaders();

	while (envp[len] != NULL)
		len++;
	// len += headers.size();
	_env = (char **)malloc(sizeof(char *) * (len + 4));
	len = 0;
	while (envp[len] != NULL) {
		_env[len] = strdup(envp[len]);
		len++;
	}
	_env[len] = strdup("REQUEST_METHOD=POST");
	tmp = "CONTENT_LENGTH=" + headers["Content-Length"];
	_env[len + 1] = strdup(tmp.c_str());
	tmp = "CONTENT_TYPE=" + headers["Content-Type"];
	_env[len + 2] = strdup(tmp.c_str());
	// _env[len + 2] = strdup("QUERY_STRING")
	_env[len + 3] = NULL;

	bodyLength = std::stoi(headers["Content-Length"]);
}

// std::ostream&   operator<<(std::ostream& out, cgiHandler const &obj) {

// }