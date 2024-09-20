#include <sys/types.h>
#include <csignal>
#include <unistd.h>
#include "server.hpp"
#include "response.hpp"
#include "request.hpp"
#include "cgiHandler.hpp"
#include "colours.hpp"

void	exit_error(const char *str, int fd, int do_close);

Server::Server() {
	addr_len = sizeof(address);
	opt = 1;
}

Server::~Server() {}

int	Server::create(void) {
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("Opening listening socket failed");
		exit(1);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	if (bind(fd, (struct sockaddr*)&address, addr_len) < 0) {
		perror("Binding listening socket to port failed");
		close(fd);
		exit(1);
	}
	return (0);
}

int	Server::run(char **env) {
	if (listen(fd, 3) < 0)
		exit_error("listening on listen_sock failed", fd, 1);
	while (1) {
		Request request;
		Response response;

		request.connect_fd = accept(fd, (struct sockaddr*)&address, &addr_len);
		if (request.connect_fd < 0)
			exit_error("accept incoming connection on connect_sock failed", fd, 1);
		request.readRequest(fd);
		if (!request.extension.compare(".quit"))
		{
			close(request.connect_fd);
			close(fd);
			exit(1);
		}
		if (!request.method.compare("POST"))
		{
			std::cout << request << std::endl;
			cgiHandler cgi(request, response, env);
			cgi.execute();
			request.resource.resize(20); //! change this to actually resize to the correct new size, not just a placeholder hardcoded value
			std::cout << LILA "---CGI raw response---\n" RST << cgi.rawResponse << LILA "---CGI raw response---" RST << std::endl;
			if (!request.getURL().compare("form.cgi")) {
				if (cgi.rawResponse.compare("Leslie"))
					request.resource.replace(request.resource.begin(), request.resource.end(), "www/html/BOO.html");
				else
					request.resource.replace(request.resource.begin(), request.resource.end(), "www/html/YAY.html");
			}
			else
			{
				if (cgi.rawResponse.compare("OK"))
					request.resource.replace(request.resource.begin(), request.resource.end(), "www/html/uploadBOO.html");
				else
					request.resource.replace(request.resource.begin(), request.resource.end(), "www/html/uploadYAY.html");
			}
		}
		response.create(request);
		// std::cout << response << std::endl;
		sendResponse(request.connect_fd, response);
	}
	close(fd);
	return (0);
}

void	Server::sendResponse(int connect_fd, Response &response) {
	send(connect_fd, response.statusLine.c_str(), response.statusLine.length(), 0);
	send(connect_fd, response.contentType.c_str(), response.contentType.length(), 0);
	send(connect_fd, response.contentLength.c_str(), response.contentLength.length(), 0);
	send(connect_fd, response.entityBody.c_str(), response.entityBody.length(), 0);
	close(connect_fd);
}


// #include <sys/types.h>
// #include <csignal>
// #include <unistd.h>
// #include "server.hpp"
// #include "response.hpp"
// #include "request.hpp"
// #include "cgiHandler.hpp"
// #include "colours.hpp"

// void	exit_error(const char *str, int fd, int do_close);

// Server::Server() {
// 	addr_len = sizeof(address);
// 	addr = NULL;
// 	opt = 1;
// 	_name = "momohost";
// 	_host = "127.0.0.1";
// 	_port = "8080";
// }

// Server::~Server() {}

// int	Server::create(void) {

// 	if (!_configServer())
// 		return (0);
// 	if (fd > 0)
// 		close(fd);
// 	return (1);
// }

// int	Server::run(char **env) {
// 	if (listen(fd, 3) < 0)
// 		exit_error("listening on listen_sock failed", fd, 1);
// 	while (1) {
// 		Request request;
// 		Response response;

// 		request.connect_fd = accept(fd, addr->ai_addr, &addr->ai_addrlen);
// 		if (request.connect_fd < 0)
// 			exit_error("accept incoming connection on connect_sock failed", fd, 1);
// 		request.readRequest(fd);
// 		if (!request.extension.compare(".quit"))
// 		{
// 			close(request.connect_fd);
// 			close(fd);
// 			exit(1);
// 		}
// 		if (!request.method.compare("POST"))
// 		{
// 			std::cout << request << std::endl;
// 			cgiHandler cgi(request, response, env);
// 			cgi.execute();
// 			request.resource.resize(20); //! change this to actually resize to the correct new size, not just a placeholder hardcoded value
// 			std::cout << LILA "---CGI raw response---\n" RST << cgi.rawResponse << LILA "---CGI raw response---" RST << std::endl;
// 			if (!request.getURL().compare("form.cgi")) {
// 				if (cgi.rawResponse.compare("Leslie"))
// 					request.resource.replace(request.resource.begin(), request.resource.end(), "www/html/BOO.html");
// 				else
// 					request.resource.replace(request.resource.begin(), request.resource.end(), "www/html/YAY.html");
// 			}
// 			else
// 			{
// 				if (cgi.rawResponse.compare("OK"))
// 					request.resource.replace(request.resource.begin(), request.resource.end(), "www/html/uploadBOO.html");
// 				else
// 					request.resource.replace(request.resource.begin(), request.resource.end(), "www/html/uploadYAY.html");
// 			}
// 		}
// 		response.create(request);
// 		// std::cout << response << std::endl;
// 		sendResponse(request.connect_fd, response);
// 	}
// 	close(fd);
// 	freeaddrinfo(addr);
// 	return (0);
// }

// void	Server::sendResponse(int connect_fd, Response &response) {
// 	send(connect_fd, response.statusLine.c_str(), response.statusLine.length(), 0);
// 	send(connect_fd, response.contentType.c_str(), response.contentType.length(), 0);
// 	send(connect_fd, response.contentLength.c_str(), response.contentLength.length(), 0);
// 	send(connect_fd, response.entityBody.c_str(), response.entityBody.length(), 0);
// 	close(connect_fd);
// }
