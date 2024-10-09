#include "server.hpp"
#include <iostream>

int	Server::_configServer(void) {

	struct addrinfo	hints, *result;
	int	status;
	fd = -1;

	//Setting up hints to get address information
	std::memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	//ChatGPT says 'get address information' but I think this is also where we connect the servername to the hostname
	status = getaddrinfo("momohost", "8080", &hints, &addr);
	if (status) {
		std::cout << "getaddrinfo error: " << gai_strerror(status) << std::endl;
		return (1);
	}

	// first just had AF_INET, SOCK_STREAM, 0 straight into the socket function call, now it's doing the exact same thing
	// but by accessing those macros through the result struct. Which is ridiculous but i'm just following chatGPT to the letter
	// and I can change it once I get it working the way I want.
	fd = socket(addr->ai_family, addr->ai_socktype, 0);
	if (fd < 0) {
		perror("Opening listening socket failed");
		return (1);
	}

	//bind that shit
	if (bind(fd, addr->ai_addr, addr->ai_addrlen) < 0) {
		perror("Binding listening socket to port failed");
		return (1);
	}

	/* OLD WAY OF BINDING
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	if (bind(fd, (struct sockaddr*)&address, addr_len) < 0) {
		perror("Binding listening socket to port failed");
		return (1);
	}
	*/
	return (0);
}

// #include "server.hpp"
// #include <iostream>

// int	Server::_configServer(void) {

// 	struct addrinfo	hints, *result;
// 	int	status;
// 	fd = -1;

// 	//Setting up hints to get address information
// 	std::memset(&hints, 0, sizeof(struct addrinfo));
// 	hints.ai_family = AF_INET;
// 	hints.ai_socktype = SOCK_STREAM;


// 	//ChatGPT says 'get address information' but I think this is also where we connect the servername to the hostname
// 	// status = getaddrinfo("momohost", "8080", &hints, &addr);
// 	// if (status) {
// 	// 	std::cout << "getaddrinfo error: " << gai_strerror(status) << std::endl;
// 	// 	return (1);
// 	// }

// 	// first just had AF_INET, SOCK_STREAM, 0 straight into the socket function call, now it's doing the exact same thing
// 	// but by accessing those macros through the result struct. Which is ridiculous but i'm just following chatGPT to the letter
// 	// and I can change it once I get it working the way I want.
// 	fd = socket(addr->ai_family, addr->ai_socktype, 0);
// 	if (fd < 0) {
// 		perror("Opening listening socket failed");
// 		return (1);
// 	}

// 	// //bind that shit
// 	// if (bind(fd, addr->ai_addr, addr->ai_addrlen) < 0) {
// 	// 	perror("Binding listening socket to port failed");
// 	// 	return (1);
// 	// }

// 	// /* OLD WAY OF BINDING
// 	address.sin_family = AF_INET;
// 	address.sin_addr.s_addr = INADDR_ANY;
// 	address.sin_port = htons(PORT);
// 	if (bind(fd, (struct sockaddr*)&address, addr_len) < 0) {
// 		perror("Binding listening socket to port failed");
// 		return (1);
// 	}
// 	// */
// 	return (0);
// }