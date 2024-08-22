// #include <vector>
// #include <sys/socket.h>
// #include <sys/epoll.h>
// #include <arpa/inet.h>
// #include <fcntl.h>


/*
MAIN DIFF: POLL()-SELECT() vs EPOLL()

	Scalability is the main problem when dealing with a huge amount of connection TCP
	to a server.
	
	Poll and Select generate an array of file descriptor of interest, this array is then
	passed to and copied by the kernel. The kernel will then go through all the fds to
	see whether there are any events available with file's poll() operation.
	
	Epoll() instead create an instance Epoll and register into it fds of interest.
	The epoll instance monitors the registered fds and report events to the application
	when asked by the epoll_wait() call.
	This become more efficient when the (amount fds with events)/(amount fds monitored)
	ratio is relatively small.
 
*/
#include "serverClass.hpp"
#include "connectionClass.hpp"

template <typename T>
void epollLoop(int epollFd, struct epoll_event &events, std::vector<T> &servers);

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
	fcntl(server.socket, F_SETFL, O_NONBLOCK); //flagged non blocking //check return of fcntl
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
	//epoll_create return a fd to a new epoll kernel data struct
	epollFd = epoll_create(1); //from linux 2.6.8 parameter size is ignored because epoll data struct is dinamically resized
	if (epollFd == -1)
	{
		std::cerr << "Failed to create epoll instance" << std::endl;
		//close all server sockets
		//error or exception
	}
	
	//THIS ONE INTO A FUNCTION
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
	//close(epollFd);
	epollLoop(epollFd, events, servers);
}



/*
*****function loop to handle epoll events and create new connections

-epoll_wait
-accept

RETURN EPOLL_WAIT()
       On success, epoll_wait() returns the number of file descriptors
       ready for the requested I/O operation, or zero if no file
       descriptor became ready during the requested timeout
       milliseconds.  On failure, epoll_wait() returns -1 and errno is
       set to indicate the error.

*/

/*create a class for connections????? with request handlers????*/

template <typename T>
void epollLoop(int &epollFd, struct epoll_event *events, std::vector<T> &servers)
{
	std::vector<Connection> connections;
	int nfds, connSock;
	while (true)
	{
		nfds = epoll_wait(epollFd, events, MAX_EVENTS, 0); //timeout at zero, no waiting for events on fds
		if (nfds == -1)
		{
			std::cerr << "failed epoll wait" << std::endl;
			//error or exception
		}
		for (int n = 0; n < nfds; n++)
		{
			typename std::vector<T>::iterator servIt = servers.begin();
			for(; servIt < servers.end(); ++servIt)
			{
				//check if EPOLLIN or EPOLLOUT to know what action to perform
				if(events[n].data.fd == (*servIt).socket)
				{
				//add new socket to epoll instance
					//accept connection and add it to connections vector
					Connection new_conn;
					new_conn.addrLen = (socklen_t *)sizeof(new_conn.addr); //client address
					new_conn.socket = accept(((*servIt).socket), (struct sockaddr*)&new_conn.addr, new_conn.addrLen);
					if (new_conn.socket == -1)
					{
						std::cerr << "Error accepting connection" << std::endl;
						//error or exception
						//check if errno is EAGAIN or EWOULDBLOCK for not complete data transfer
						
					}
					//set non blocking
					//add new_conn to epoll instance
				}
				else
				{
					//process connections
				}
			}
		}
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
	
	exit(0);
}



/*
void make_socket_non_blocking(int socket_fd)
{
    int flags;




    flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("Couldn't get socket flags");
        exit(1);
    }



    flags |= O_NONBLOCK;
    if (fcntl(socket_fd, F_SETFL, flags) == -1) {
        perror("Couldn't set socket flags");
        exit(-1);
    }
}

*/