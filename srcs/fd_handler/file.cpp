#include "file.hpp"


File::File(std::string filename, Webserver* ptr, Client* client): _main(ptr), _client(client), _buff(""), _file_size(0)
{
	reset_last_activity();
	_fd = open(filename.c_str(), O_RDONLY);
	if (_fd < 0)
		throw WebservException("Failed to open: " + std::string(strerror(errno)));
	if (pipe(_pipe) == -1)
		throw WebservException("Failed to pipe: " + std::string(strerror(errno)));
    make_socket_non_blocking(_fd);
	_inFd = dup(_pipe[0]);
	_outFd = dup(_pipe[1]);
	if (_outFd == -1 || _inFd == -1)
        	throw WebservException("Failed to dup: " + std::string(strerror(errno)));
	close(_pipe[1]);
	close(_pipe[0]);
	int flags;
	flags |= O_NONBLOCK;
    if (fcntl(_inFd, F_SETFL, flags) == -1) 
		throw WebservException("Failed to fcntl set_flag: " + std::string(strerror(errno)));
	if (fcntl(_outFd, F_SETFL, flags) == -1) 
		throw WebservException("Failed to fcntl set_flag: " + std::string(strerror(errno)));
    _main->addFdToPoll(_inFd, _main->get_EpollFd(FILES), EPOLLIN);
    _main->addFdToMap(_inFd, this);
	_main->addFdToPoll(_outFd, _main->get_EpollFd(FILES), EPOLLOUT);
    _main->addFdToMap(_outFd, this);
}

File::~File()
{
	close(_fd);
}

void File::input()
{
	ssize_t bytes_r = read(_inFd, _buff, MAX_BUFF);
	if (bytes_r < 0)
		throw WebservException("Failed to read pipe in");
	else if (bytes_r == 0)
	{
		_client->status.clear();
		_main->removeFd(_inFd, FILES, 1);
	}
	else
	{
		_client->file_content.append(_buff, bytes_r);
		if (_client->file_content.size() == (size_t)_file_size)
		{
			_client->status.clear();
			_main->removeFd(_inFd, FILES, 1);
		}
	}
}

void File::output()
{
	ssize_t bytes_r = read(_fd, _buff, MAX_BUFF);
	if (bytes_r < 0)
		throw WebservException("Failed to read file in file_handler");
	else if (bytes_r == 0)
	{
		_main->removeFd(_outFd, FILES, 0); //remove the fd but not the handler
	}
	else
	{
		ssize_t bytes_w = write(_outFd, _buff, bytes_r);
		if (bytes_w < 0)
			throw WebservException("Failed to write pipe out");
		else if (bytes_w >= 0)
			_file_size += bytes_w;
	}
}

int File::get_inFd()
{
    return this->_inFd;
}

int File::get_outFd()
{
    return this->_outFd;
}