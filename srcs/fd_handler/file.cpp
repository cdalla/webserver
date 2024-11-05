#include "file.hpp"

File::File(int fd, Webserver* ptr): _main(ptr)
{
    this->set_fd(fd);
    make_socket_non_blocking(_fd);
    _main->addFdToPoll(_fd, _main->get_EpollFd(FILES), EPOLLIN);
    _main->addFdToMap(_fd, this);
}

bool File::consume(int event_type)
{
    (void)event_type;
    reset_last_activity();
    char buff[MAX_BUFF];
    ssize_t bytes_read;
    do
    {
        bytes_read = read(_fd, buff, MAX_BUFF);
        buff[1000] = '\0';
        _client->file_content.append(buff, bytes_read);
    }
    while(bytes_read > 0);
    if (bytes_read < 0)
        throw WebservException("Failed to read: " + std::string(strerror(errno)));

   return false;
}


/*
void responseHandler::_handlePage(std::string path)
{

    if (_client->request.method == "POST" || _client->request.method == "DELETE")
	{
		_handleError(405);
		return;
	}

    //////////////////////////////////////////
    PART CHANGED BY CARLO
    if (_client->file_content.empty())
    {
	    int file_fd = open(path.c_str(), O_RDONLY);
	    if (file_fd == -1)
	    {
		    if (errno == EACCES)
			    _handleError(403); // Permission Denied
		    else
			    _handleError(404); // File Not Found
		    return;
	    }
        File* file = new File(file_fd, _main);
        return ;
    }
    _body.append(_client->file_content);
    main->removeFd(file_fd, FILES);
    
    //////////////////////////////////////////

	_determineType(path);//idk if u need this
	_response = "200 OK\r\n";
	_response.append("Content-Type: ");
	_response.append(_content_type);
	_response.append("\r\n");
	_response.append("Content-Length: ");
	_response.append(std::to_string(_body.length()));
	_response.append("\r\n\r\n");
	_response.append(_body);
}
*/