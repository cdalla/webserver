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
    {
        //FATAL
        perror("Couldn't get socket flags");
        exit(1);
    }

    flags |= O_NONBLOCK;
    if (fcntl(socket_fd, F_SETFL, flags) == -1) 
    {
        //FATAL
        perror("Couldn't set socket flags");
        exit(-1);
    }
}

// std::string		get_URI_prefix(std::string const &URI) {

// 	std::string::size_type pos = URI.find_last_of('/');
// 	if (pos == npos)
// 		throw std::runtime_error("Error extracting prefix from URI: no forward slash present"); // i don't see how we would ever even make it here

// 	return (URI.substr(0, pos));
// }