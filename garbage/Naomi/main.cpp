#include <iostream>
#include <unistd.h>
#include <netinet/in.h> //for the definition of sockaddr_in (for more info see https://learn.microsoft.com/en-us/windows/win32/api/ws2def/ns-ws2def-sockaddr_in)
#include <csignal>

#include "server.hpp"
#include "request.hpp"
#include "response.hpp"

void		exit_error(const char *str, int fd, int do_close);
int			listen_sock;

// void		signalHandler(int signal_num) {
// 	close(listen_sock);
// 	std::cout << "Program interrupted with SIGTERM" << std::endl;
// 	exit(0);
// }

int	main(int argc, char **argv, char **envp) {
	Server				server;

	if (server.create())
		exit_error("Error creating server", 0, 0);
	return (server.run(envp));
}

void	exit_error(const char *str, int fd, int do_close) {
	if (do_close)
		close(fd);
	perror(str);
	exit(1);
}
