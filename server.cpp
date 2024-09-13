#include "server.hpp"

Server::Server()
{
    //fill info to set the socket
    //fill with conf info
    createSocket();
}

void Server::createSocket()
{
    	//CREATE SOCKET
	socket = socket(AF_INET, SOCK_STREAM, 0);
	if (!server.socket)
	{
		std::cerr << "Failed to create socket!" << std::endl;
		//error or exception, change return type funct
	}
	
	int sockoption = 1;
	//SET NON BLOCKING AND REUSABLE ADDR
	//	SOL_SOCKET used for option protocol indipendent, reuse address as 1 = true
	setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &sockoption, sizeof(sockoption)); //check error
	make_socket_non_blocking(socket);

	//BIND TO ADDRESS AND PORT
	address.sin_family = AF_INET; //ipv4
	address.sin_addr.s_addr = INADDR_ANY; //listen on any address
	address.sin_port = htons(server.port); // convert port value
	if (bind(socket, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		std::cerr << "Failed to bind socket!" << std::endl;
		close(socket);
		//error or exception, change return type funct
	}
	
	//LISTEN FOR INCOMING CONNECTIONS
	if (listen(socket, MAX_CONNECTIONS) < 0)
	{
		std::cerr << "Failed to listen on socket!" << std::endl;
		//error or exception, change return type function
	}
}