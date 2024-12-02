#include "webserver.hpp"
#include "utils.hpp"

 //flagged non blocking //check return of fcntl
	/*
		non-blocking I/O means when reading an fd, if nothing is available,
		return an error immediately rather than waiting/blocking until data is available.
	*/

void make_socket_non_blocking(int socket_fd)
{
    int flags;

    flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1)
		throw WebservException("Failed to fcntl get_flag: " + std::string(strerror(errno)));

    flags |= O_NONBLOCK;
    if (fcntl(socket_fd, F_SETFL, flags) == -1) 
		throw WebservException("Failed to fcntl set_flag: " + std::string(strerror(errno)));
}

void print_msg(std::string param)
{
    std::cout << GRN << "Webserver: " << param << RST << std::endl;
}

void print_error(std::string param)
{
    std::cerr << RED << "Webserver_error: " << param << RST << std::endl;
}

// std::string		get_URI_prefix(std::string const &URI) {

// 	std::string::size_type pos = URI.find_last_of('/');
// 	if (pos == npos)
// 		throw std::runtime_error("Error extracting prefix from URI: no forward slash present"); // i don't see how we would ever even make it here

// 	return (URI.substr(0, pos));

// }