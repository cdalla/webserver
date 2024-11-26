#include "cgi.hpp"


Cgi::Cgi(Webserver *ptr, const char *script, char *const *env, const char *body, Client *client) : _main(ptr), _pipeIn{-1, -1}, _pipeOut{-1, -1}, _env(env), _script(script), _body(body), _pos(0), _writeFinished(false), _client(client)
{
    print_msg("cgi handler consttructor");
    reset_last_activity();
    if (pipe(_pipeIn) == -1 || pipe(_pipeOut) == -1)
        throw WebservException("Failed to pipe: " + std::string(strerror(errno)));
    _pid = fork();
    if (_pid < 0)
        throw WebservException("Failed to fork: " + std::string(strerror(errno)));
    else if (_pid == 0)
    {
        execute_child();
    }
	_outFd = dup(_pipeIn[1]);
	_inFd = dup(_pipeOut[0]);
	if (_outFd == -1 || _inFd == -1)
		std::cout << "PORCA TROIA!!!!" << std::endl;
	close(_pipeIn[1]);
	close(_pipeOut[0]);
	std::cout << "script: " << script << std::endl;
	make_socket_non_blocking(_outFd);
	make_socket_non_blocking(_inFd);
    _main->addFdToPoll(_outFd, _main->get_EpollFd(FILES), EPOLLOUT);
    _main->addFdToPoll(_inFd, _main->get_EpollFd(FILES), EPOLLIN | EPOLLHUP );
    _main->addFdToMap(_outFd, this);
    _main->addFdToMap(_inFd, this);
    close(_pipeIn[0]);
    close(_pipeOut[1]);
    std::cout << "CGI inFd = " << _inFd << ", CGI outFd = " << _outFd << std::endl;
}

Cgi::~Cgi()
{
    // check destructor
    print_msg("CGI destructor");
    int status;
    waitpid(_pid, &status, 0);
    if (WIFEXITED(status))
    {
        status = WEXITSTATUS(status);
		_client->request.error = status;
    }
}

bool Cgi::consume(int event_type)
{
    if (event_type == IN)
    {
        reset_last_activity();
        char buff[MAX_BUFF];
        std::memset(buff, '\0', MAX_BUFF);
        ssize_t bytes = read(_inFd, buff, MAX_BUFF);
        if (bytes == 0)
        {
            _client->cgi_result = _cgi_result;
            _main->removeFd(_inFd, FILES, 0);
            _main->removeFd(_outFd, FILES, 1);
        }
        else if (bytes < 0)
            throw WebservException("Failed to read: " + std::string(strerror(errno)));
        else
            _cgi_result.append(buff, bytes);
    }
    else if (event_type == OUT)
    {

        reset_last_activity();
        if (_writeFinished == true)
            return false;
        std::string toSend = _body.substr(_pos, MAX_BUFF);
        ssize_t bytes = write(_outFd, toSend.c_str(), MAX_BUFF);
        if (bytes < 0)
            throw WebservException("Failed to write: " + std::string(strerror(errno)));
        else if (bytes == 0)
        {
            _writeFinished = true;
        }
        else
        {
            _pos += bytes;
        }
    }

    return false;
}

void Cgi::input()
{
    print_msg("CGI input");
        reset_last_activity();
        char buff[MAX_BUFF];
        std::memset(buff, '\0', MAX_BUFF);
        ssize_t bytes = read(_inFd, buff, MAX_BUFF);
		std::cout << "bytes read: " << bytes << std::endl;
        if (bytes == 0)
        {
			print_msg("read zero bytes in cgi");
            _client->file_content = _cgi_result;
            _client->status.clear();
            _main->removeFd(_inFd, FILES, 1);
        }
        else if (bytes < 0)
            throw WebservException("Failed to read: " + std::string(strerror(errno)));
        else
		{
			std::cout << "cgi buff: \n:" << buff << std::endl;
            _client->file_content.append(buff, bytes);
		}
}

void Cgi::output()
{
    print_msg("CGI output");
    std::cout << "_body in cgi output: \n" << _body << std::endl; 
        reset_last_activity();
        if (_writeFinished == true)
            return;
        if (_body.empty())
        {
            _writeFinished = true;
            _main->removeFd(_outFd, FILES, 0);
            return;
        }
        std::string toSend = _body.substr(_pos, MAX_BUFF);
        ssize_t bytes = write(_outFd, toSend.c_str(), toSend.size());
        if (bytes < 0)
            throw WebservException("Failed to write: " + std::string(strerror(errno)));
        else if (bytes == 0)
        {
            _writeFinished = true;
            _main->removeFd(_outFd, FILES, 0);
        }
        else
        {
            _pos += bytes;
        }
}



void Cgi::hangup()
{
    int exitstatus;
    if (waitpid(_pid, &exitstatus, WNOHANG) == 0) //child has not changed state yet
        return;
    
	waitpid(_pid, &exitstatus, 0);

	print_msg("clearing cgi");
	_client->status.clear();
	_main->removeFd(_inFd, FILES, 1);
	switch(exitstatus) 
	{
		case 137:
			this->_client->request.error = 504;
            this->_client->file_content.clear();
			break ;
		case 3328:
			this->_client->request.error = 403;
			this->_client->file_content.clear();
			break ;
		case 0:
			break ;
		default:
			this->_client->request.error = 502;
            this->_client->file_content.clear();
	}
}



void Cgi::execute_child()
{
    close(_pipeIn[1]);
    close(_pipeOut[0]);
	_script = "/home/cdalla/webserver/www/cgi-bin/upload.py";
    char *argv[] = {(char *)_script, (char *)_script, NULL};
    if (dup2(_pipeIn[0], STDIN_FILENO) < 0)
    {
        close(_pipeIn[0]);
        close(_pipeOut[1]);
        exit(errno);
    }
    close(_pipeIn[0]);
    if (dup2(_pipeOut[1], STDOUT_FILENO) < 0)
    {
        close(STDIN_FILENO);
        close(_pipeOut[1]);
        exit(errno);
    }
    close(_pipeOut[1]);
	int error = execve(_script, argv, _env);
	std::cerr << "after ex " << _script << std::endl;
	std::cerr << "Error: " << error << std::endl;
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    exit(error);
}