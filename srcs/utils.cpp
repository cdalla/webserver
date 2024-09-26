
#include "webserver.hpp"

 //flagged non blocking //check return of fcntl
	/*
		non-blocking I/O means when reading an fd, if nothing is available,
		return an error immediately rather than waiting/blocking until data is available.
	*/

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