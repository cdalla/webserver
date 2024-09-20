#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h> //for the definition of sockaddr_in (for more info see https://learn.microsoft.com/en-us/windows/win32/api/ws2def/ns-ws2def-sockaddr_in)
#include <string>

#include "request.hpp"

#define PORT 8080

// adhv deze tutorial: https://www.geeksforgeeks.org/socket-programming-cc/

void	exit_error(const char *str, int fd, int do_close);


int	main(void) {

	/* Gonna need to store our incoming message (what will eventually be the HTTP request). */
	char	buf[1024];
	char	msg[] = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 474\n\n<!DOCTYPE html>\n<html lang=\"en\">\n<head><style>body { font-family: Arial; }</style><title>Fucking dope HTML page</title></head><body><h1>YOOOOOOOOOOOOO</h1><p>This is a story<br>All about how<br>My life got flip turned upside down<br>And I'd like to take a minute just sit right there<br>And tell you how I became the prince of a town called Bel-Air<br><br><img src=\"/Users/naomisterk/codam/webbieservie/www/img/lute.jpg\" width=\"407\" height=\"402\"></p></body></html>";
	char	img[] = "HTTP/1.1 200 OK\nContent-Type: image/jpeg\n\n<!DOCTYPE html>\n<html><body></body></html>";
	pid_t	fd;
	/** To store the return from our read operation on the connect_sock */
	int		valread = 0;

	/**
	 * Need to set up a listening socket (listen_sock) to listen for incoming connection requests. Once a request
	 * comes in and is accepted, a new socket (connect_sock) is created on the same port as the listening socket.
	 * The sockets are accessed through file descriptors, which is why these variables are of type integer.
	 */
	int		listen_sock, connect_sock;

	/**
	 * The variable server_address is needed to bind a port to the socket, and to set the socket options.
	 * We are going to specify what type of socket it is, and include the network bytes of the port
	 * (cause 8080 doesn't mean shit). The network bytes of the port are obtained with htons().
	 */
	struct sockaddr_in	server_address;
	socklen_t			addr_len = sizeof(server_address);

	/**
	 * The variable 'opt' will be passed to setsockopt as parameter option_value (a pointer to this
	 * variable, to be exact). Its length is passed as the parameter option_len. These two parameters
	 * are used to access option values for setsockopt().
	 */

	int				opt = 1;

	// ------> THIS IS WHERE THE CODING STARTS <------

	/**
	 * Create a socket(domain, type, protocol)
	 * 
	 * AF_INET means we want a socket in the domain 'internet address'.
	 * SOCK_STREAM means we want the type of this socket to be bidirectional connection-mode (and some more shit: https://pubs.opengroup.org/onlinepubs/7908799/xns/socket.html)
	 * Right now we have protocol specified to 0 which causes socket() to use an unspecified default protocol. Cause I don't know which one to use.
	 */
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock < 0)
		exit_error("Opening listening socket failed", 0, 0);
	
	/**
	 * Setting socket options(socket, level, option_name, *option_value, option_len)
	 * 
	 * listen_sock is the socket we want to be setting the options for
	 * we want to manipulate options at the socket level, daarom SOL_SOCKET
	 * I just followed the tutorial for option_name, I don't get it but not a priority right now.
	 * 
	 */
	// if (setsockopt(listen_sock, SOL_SOCKET, 0, &opt, sizeof(opt)))
	// 	exit_error("setsockopt problemmmm", listen_sock, 1);
	
	server_address.sin_family = AF_INET; // address family for the transport address. Should always be set to AF_INET
	server_address.sin_addr.s_addr = INADDR_ANY; // IN_ADDR structure that contains an IPv4 transport address (https://learn.microsoft.com/en-us/windows/win32/api/winsock2/ns-winsock2-in_addr)
	server_address.sin_port = htons(PORT); // converts the port into network bytes

	/**
	 * Binding the socket to port 8080
	 * 
	 * bind takes a pointer to a sockaddr struct, so we need to point to our sockaddr_in struct and cast that to a pointer to a sockaddr struct
	 */
	if (bind(listen_sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
		exit_error("binding listen_sock to port failed", listen_sock, 1);
	
	if (listen(listen_sock, 3) < 0) // the number indicates the max queue length (so in this case 3)
		exit_error("listening on listen_sock failed", listen_sock, 1);
	
	/**
	 * ACCEPTING INCOMING CONNECTION AND READING DATA
	 * 
	 */
	while (1)
	{
		connect_sock = accept(listen_sock, (struct sockaddr*)&server_address, &addr_len);
		if (connect_sock < 0)
			exit_error("accept incoming connection on connect_sock failed", listen_sock, 1);
		valread = read(connect_sock, buf, 1023);
		if (valread < 0)
			exit_error("reading from connect_sock failed", listen_sock, 1);
		buf[valread] = '\0';
		printf("Message from client: %s\n", buf);
		fd = fork();
		// send(connect_sock, msg, strlen(msg), 0);
		close(connect_sock);
		break ;
	}
	close(listen_sock);
	return (0);
}

void	execute_cgi(int connect_sock)
{

}

void	exit_error(const char *str, int fd, int do_close)
{
	if (do_close)
		close(fd);
	perror(str);
	exit(1);
}
