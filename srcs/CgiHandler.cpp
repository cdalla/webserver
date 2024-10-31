#include "CgiHandler.hpp"
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <signal.h>
#include <time.h>
#include <chrono>
#include <iostream>
#include <errno.h>
#define MAX_TIME 60000

static void st_close_fd(int fd_1, int fd_2)
{
	close(fd_1);
	close(fd_2);
}

CgiHandler::~CgiHandler(void){}

CgiHandler::CgiHandler(const char *script, char * const *env, const char *body)
{
	
	if (initialize_pipe() < 0)
		return ;
	
	int pid = fork();
	switch(pid) {
		case -1:
			_status_code = 500;
			_status_mess = strerror(errno);
			break ;
		case 0:
			child_exe(script, env);
			break ;
		default:
			parent_exe(body, pid);
	}

}

void CgiHandler::parent_exe(const char *body, pid_t pid)
{
	int epoll_fd = epoll_create(1);
    if (epoll_fd == -1) {
        _status_code = 500;
        _status_mess = "Failed to create epoll instance";
        return;
    }

    struct epoll_event event, events[MAX_EVENTS];
    event.events = EPOLLOUT | EPOLLET;
    event.data.fd = _fd_in[1];
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _fd_in[1], &event) == -1) {
        _status_code = 500;
        _status_mess = "Failed to add fd_in to epoll";
        close(epoll_fd);
        return;
    }

    event.events = EPOLLIN | EPOLLET;
    event.data.fd = _fd_out[0];
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _fd_out[0], &event) == -1) {
        _status_code = 500;
        _status_mess = "Failed to add fd_out to epoll";
        close(epoll_fd);
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();
    st_close_fd(_fd_in[0], _fd_out[1]);
    bool read_finished = false;
    bool write_finished = false;
    const char* write_ptr = body;
    size_t remaining = strlen(body);

	char buffer[200];
	memset(buffer, '\0', 200);
	while ((!read_finished || !write_finished))
	{
		 for (int i = 0; i < num_events; i++) 
        { 
            if (events[i].data.fd == _fd_out[0] && !read_finished)
            {
                while (true) {
                    int bytes_read = read(_fd_out[0], buffer, sizeof(buffer));
                    if (bytes_read > 0) {
                        _response.append(buffer, bytes_read);
                    } else if (bytes_read == 0) {
                        read_finished = true;
                        break;
                    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        break;
                    } else {
                        _status_code = 500;
                        _status_mess = "Read error: " + std::string(strerror(errno));
                        read_finished = true;
                        break;
                    }
                }
            }
            if (events[i].data.fd == _fd_in[1] && !write_finished)
            {
                while (remaining > 0) {
                    int bytes_written = write(_fd_in[1], write_ptr, remaining);
                    if (bytes_written > 0) {
                        write_ptr += bytes_written;
                        remaining -= bytes_written;
                    } else if (bytes_written == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        } else {
                            _status_code = 500;
                            _status_mess = "Write error: " + std::string(strerror(errno));
                            write_finished = true;
                            break;
                        }
                    }
                }
                if (remaining == 0) {
                    close(_fd_in[1]);
                    write_finished = true;
                }
            }    
        }

		auto corrent = std::chrono::high_resolution_clock::now();
		std::chrono::milliseconds time_passed = std::chrono::duration_cast<std::chrono::milliseconds>(corrent - start);
		if (time_passed.count() > MAX_TIME)
		{
			kill(pid, SIGKILL);
			break ;
		}
	}

    close(epoll_fd);
    if (_fd_in[1])
        close(_fd_in[1]);
    close(_fd_out[0]);

	int status;
	waitpid(pid, &status, 0);
	switch(status) 
	{
		case 137:
			_status_code = 504;
			_status_mess = "Gateway Time-out";
			break ;
		case 3328:
			_status_code = 403;
			_status_mess = "Forbidden";
			break ;
		case 0:
			_status_code = 200;
			_status_mess = "OK";
			break ;
		default:
			_status_code = 502;
			_status_mess = "Bad Gateway";
	}
}

void CgiHandler::child_exe(const char *script, char * const *env)
{
	close(_fd_in[1]);
	close(_fd_out[0]);
	char* argv[]={(char*)script, (char*)script,  NULL};
	
	if (dup2(_fd_in[0], STDIN_FILENO) < 0)
	{
		st_close_fd(_fd_in[0], _fd_out[1]);
		exit(errno);
	}
	close(_fd_in[0]);
	if (dup2(_fd_out[1], STDOUT_FILENO) < 0)
	{
		st_close_fd(STDIN_FILENO, _fd_out[1]);
		exit(errno);
	}
	close(_fd_out[1]);

	execve(script, argv, env);
	int error = errno;
	st_close_fd(STDIN_FILENO, STDOUT_FILENO);
	exit (error);
}


int CgiHandler::initialize_pipe( void )
{
	if (pipe(_fd_in) < 0)
	{
		_status_code = 500;
		_status_mess = "Pipe failed: " + std::string(strerror(errno));
		return -1;
	}
	if( pipe(_fd_outs) < 0)
	{
		st_close_fd(_fd_in[0], _fd_in[1]);
		_status_code = 500;
		_status_mess = "Pipe failed: " + std::string(strerror(errno));
		return -1;
	}
	return 0;
}

std::string  CgiHandler::get_status_mess ( void ) const
{
	return _status_mess;
}
int CgiHandler::get_status_code( void ) const
{
	return _status_code;
} 

std::string CgiHandler::get_response( void ) const
{
	return _response;
}