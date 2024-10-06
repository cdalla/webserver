#ifndef SORVER_HPP
# define SORVER_HPP

// #include "response.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

#define PORT 8080

class Response;

class Sorver {
	public:
		Sorver();
		~Sorver();

		int		create(void);
		int		run(char **env);
		void	sendResponse(int connect_fd, Response &response);

		// void	handlePost(Request &request, Response &response);
		// void	signalHandler(int signal_num);

		int					fd;
		struct sockaddr_in	address;
		socklen_t 			addr_len;
		int					opt;
		std::string			buf;

	private:
};

#endif

// #ifndef SERVER_HPP
// # define SERVER_HPP

// // #include "response.hpp"
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <netdb.h> // for struct addrinfo
// #include <netinet/in.h>
// #include <string>

// #define PORT 8080

// class Response;

// class Server {
// 	public:
// 		Server();
// 		~Server();

// 		int		create(void);
// 		int		run(char **env);
// 		void	sendResponse(int connect_fd, Response &response);

// 		// void	signalHandler(int signal_num);

// 		int					fd;
// 		struct sockaddr_in	address;
// 		struct addrinfo		*addr;
// 		socklen_t 			addr_len;
// 		int					opt;
// 		std::string			buf;

// 	private:
// 		int					_configServer(void);
// 		unsigned int		_hostInt;
// 		std::string			_name;
// 		std::string			_host;
// 		std::string			_port;
// };

// #endif