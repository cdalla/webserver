#include "cgi.hpp"

Cgi::Cgi(Webserver *ptr, const char *script, char *const *env, std::string body, Client *client) : _main(ptr), _pipeIn{-1, -1}, _pipeOut{-1, -1}, _env(env), _script(script), _body(body), _pos(0), _writeFinished(false), _client(client)
{
    print_msg("cgi handler consttructor");
    reset_last_activity();
    
    //SET UP PIPES
    if (pipe(_pipeIn) == -1 || pipe(_pipeOut) == -1)
        throw WebservException("Failed to pipe: " + std::string(strerror(errno)));	
	_pid = fork();
    if (_pid < 0)
        throw WebservException("Failed to fork: " + std::string(strerror(errno)));
    else if (_pid == 0)
    {
        execute_child();
    }

    //SET UP FDs FOR EPOLL
	_outFd = dup(_pipeIn[1]);
	_inFd = dup(_pipeOut[0]);
	if (_outFd == -1 || _inFd == -1)
        	throw WebservException("Failed to dup: " + std::string(strerror(errno)));
	close(_pipeIn[1]);
	close(_pipeOut[0]);
	make_socket_non_blocking(_outFd);
	make_socket_non_blocking(_inFd);
    _main->addFdToPoll(_outFd, _main->get_EpollFd(FILES), EPOLLOUT);
    _main->addFdToPoll(_inFd, _main->get_EpollFd(FILES), EPOLLIN | EPOLLHUP );
    _main->addFdToMap(_outFd, this);
    _main->addFdToMap(_inFd, this);
    close(_pipeIn[0]);
    close(_pipeOut[1]);
	// dup2(oldin, STDIN_FILENO);
	// dup2(oldout, STDOUT_FILENO);
   // std::cout << "CGI inFd = " << _inFd << ", CGI outFd = " << _outFd << std::endl;
}

Cgi::~Cgi()
{
    print_msg("CGI destructor");
    //child still running
    int exitstatus;
    if (waitpid(_pid, &exitstatus, WNOHANG) == 0)
    {
        std::cout << "killing child process" << std::endl;
		this->_client->request.error = 502;
        this->_client->file_content.clear();
        this->_client->status.clear();
        kill(_pid, SIGQUIT);
    }
    // int status;
    
    // waitpid(_pid, &status, 0);
    // if (WIFEXITED(status))
    // {
    //     status = WEXITSTATUS(status);
	// 	_client->request.error = status;
    // }
}

void Cgi::input()
{
    print_msg("CGI input");
    //reset_last_activity();
    char buff[MAX_BUFF];
    std::memset(buff, '\0', MAX_BUFF);
    ssize_t bytes = read(_inFd, buff, MAX_BUFF);
	std::cout << "bytes read: " << bytes << std::endl;
    if (bytes == 0)
    {
		print_msg("read zero bytes in cgi");
        _client->status.clear();
        _main->removeFd(_inFd, FILES, 1);
    }
    else if (bytes < 0)
        throw WebservException("Failed to read: " + std::string(strerror(errno)));
    else
	{
		//std::cout << "cgi buff: \n:" << buff << std::endl;
        _client->file_content.append(buff, bytes);
	}
}

void Cgi::output()
{
    print_msg("CGI output");
    //std::cout << "_body in cgi output: \n" << _body << std::endl; 
    //reset_last_activity();
    if (_writeFinished == true)
        return;
    if (_body.empty())
    {
        _writeFinished = true;
        _main->removeFd(_outFd, FILES, 0);
        _outFd = -1;
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
        _outFd = -1;
		//std::cout << "pos: " << _pos << std::endl;
        print_msg("finish writing body");
    }
    else
    {
        _pos += bytes;
    }
}

void Cgi::hangup()
{
	print_msg("CGI hangup");
    int exitstatus;
    if (waitpid(_pid, &exitstatus, WNOHANG) == 0) //child has not changed state yet
        return;

	print_msg("clearing cgi");
	_client->status.clear();
	if (WIFEXITED(exitstatus))
    {
        switch((exitstatus)) 
	    {
		    case 137:
			    this->_client->request.error = 504;
                this->_client->file_content.clear();
                this->_client->status.clear();
			    break ;
		    case 3328:
			    this->_client->request.error = 403;
			    this->_client->file_content.clear();
                this->_client->status.clear();
                break ;
		    case 0:
			    break ;
            
		    default:
			    this->_client->request.error = 502;
                this->_client->file_content.clear();
                this->_client->status.clear();
                break ;
	    }
    }
	_main->removeFd(_inFd, FILES, 1);
}

void Cgi::execute_child()
{
    close(_pipeIn[1]);
    close(_pipeOut[0]);
	//_script = "/home/cdalla/webserver/www/cgi-bin/infinite.py";
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

int Cgi::get_inFd()
{
    return this->_inFd;
}

int Cgi::get_outFd()
{
    return this->_outFd;
}


//add a max timeout for cgi to prevent infinite loops
