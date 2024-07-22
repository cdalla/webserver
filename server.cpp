// #include <vector>
// #include <sys/socket.h>
// #include <sys/epoll.h>
// #include <arpa/inet.h>
// #include <fcntl.h>

#include "serverClass.hpp"


//change value according to program
#define MAX_CONNECTIONS 10
#define MAX_EVENTS 10


/*
*****function caller to set up all servers 
	-loop through as many servers to create
		and call funct to set up single one
	-epoll instance creation call
	-epoll event managing call
*/

template <typename T>
void	serverInit(std::vector<T> &servers)
{	
	typename std::vector<T>::iterator it = servers.begin();
	for (; it != servers.end(); ++it)
	{
		createSocket(*it);		
	}
	createEpollInstance(servers);
}


/*
*****function to create single socket, reusable for every server to set up

-socket + socketopt(set it reusable addr)
-fnctl (set it non blocking)
-struct sock addr.sin
-bind port to addr
-listen on socket

{
	server Class attributes
	
	struct sockaddr_in address;
	int socket;
	int port; ?? or std::string port;
	
}

*/
template <typename T>
void	createSocket(T &server)
{
	//CREATE SOCKET
	server.socket = socket(AF_INET, SOCK_STREAM, 0);
	if (!server.socket)
	{
		std::cerr << "Failed to create socket!" << std::endl;
		//error or exception, change return type funct
	}
	
	int sockoption = 1;
	//SET NON BLOCKING AND REUSABLE ADDR
	//	SOL_SOCKET used for option protocol indipendent, reuse address as 1 = true
	setsockopt(server.socket, SOL_SOCKET, SO_REUSEADDR, &sockoption, sizeof(sockoption)); //check error
	fcntl(server.socket, F_SETFL, O_NONBLOCK); //flagged non blocking
	/*
		non-blocking I/O means when reading an fd, if nothing is available,
		return an error immediately rather than waiting/blocking until data is available.
	*/
	
	//BIND TO ADDRESS AND PORT
	server.address.sin_family = AF_INET; //ipv4
	server.address.sin_addr.s_addr = INADDR_ANY; //listen on any address
	server.address.sin_port = htons(server.port); // convert port value
	if (bind(server.socket, (struct sockaddr*)&server.address, sizeof(server.address)) < 0)
	{
		std::cerr << "Failed to bind socket!" << std::endl;
		close(server.socket);
		//error or exception, change return type funct
	}
	
	//LISTEN FOR INCOMING CONNECTIONS
	if (listen(server.socket, MAX_CONNECTIONS) < 0)
	{
		std::cerr << "Failed to listen on socket!" << std::endl;
		//error or exception, change return type function
	}
}


/*
*****function to create epoll instance and add servers socket to epoll

-epoll_create
-epoll_ctl(EPOLL_CTL_ADD) -> to add all the sockets to epoll instance

*/

template <typename T>
void	 createEpollInstance(std::vector<T> &servers)
{
	int 				epollFd;
	struct epoll_event	events[MAX_EVENTS];
	
	epollFd = epoll_create(1); //from linux 2.6.8 parameter size is ignored
	if (epollFd == -1)
	{
		std::cerr << "Failed to create epoll instance" << std::endl;
		//close all server sockets
		//error or exception
	}
	
	typename std::vector<T>::iterator it = servers.begin();
	for (; it != servers.end(); ++it)
	{
		(*it).event.events = EPOLLIN;
		(*it).event.data.fd = (*it).socket;
		if (epoll_ctl(epollFd, EPOLL_CTL_ADD, (*it).socket, &(*it).event) == -1)
		{
			std::cerr << "Failed to add server socket to epoll instance" << std::endl;
			//error or exception
			//close socket and epollfd
		}
	}
	close(epollFd);
}



/*
*****function loop to handle epoll events and create new connections

-epoll_wait
-accept


*/
template <typename T>
void epollLoop(int epollFd, struct epoll_event events, std::vector<T> servers)
{
	while (true)
	{
		
	}
}





/* MAIN */
int main()
{
	
	std::vector<Server> servers;
	Server test;
	
	test.port = 8080;
	servers.push_back(test);
	
	serverInit(servers);
	
	
	std::vector<Server>::iterator it = servers.begin();
	for (; it != servers.end(); ++it)
	{
		close((*it).socket);
	}
	
	return 0;
}