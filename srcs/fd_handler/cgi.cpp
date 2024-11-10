#include "cgi.hpp"

Cgi::Cgi(Webserver *ptr, const char *script, char *const *env, const char *body, Client *client) : _main(ptr), _pipeIn{-1, -1}, _pipeOut{-1, -1}, _env(env), _script(script), _body(body), _pos(0), _writeFinished(false), _client(client)
{
    reset_last_activity();
    if (pipe(_pipeIn) == -1 || pipe(_pipeOut) == -1)
        throw WebservException("Failed to pipe: " + std::string(strerror(errno)));
    _pid = fork();
    if (_pid < 0)
        throw WebservException("Failed to fork: " + std::string(strerror(errno)));
    else if (_pid == 1)
    {
        execute_child();
    }
    _main->addFdToPoll(_pipeIn[1], _main->get_EpollFd(FILES), EPOLLOUT);
    _main->addFdToPoll(_pipeOut[0], _main->get_EpollFd(FILES), EPOLLIN); // add event in fd to poll
    _main->addFdToMap(_pipeIn[1], this);
    _main->addFdToMap(_pipeOut[0], this);
    close(_pipeIn[0]);
    close(_pipeOut[1]);
}

Cgi::~Cgi()
{
    // check destructor
    int status;
    waitpid(_pid, &status, 0);
    if (WIFEXITED(status))
    {
        status = WEXITSTATUS(status);
    }
}

bool Cgi::consume(int event_type)
{
    if (event_type == IN)
    {
        reset_last_activity();
        char buff[MAX_BUFF];
        std::memset(buff, '\0', MAX_BUFF);
        ssize_t bytes = read(_pipeOut[0], buff, MAX_BUFF);
        if (bytes == 0)
        {
            _client->cgi_result = _cgi_result;
            _main->removeFd(_pipeOut[0], FILES);
            _main->removeFd(_pipeIn[1], FILES);
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
        ssize_t bytes = write(_pipeIn[1], toSend.c_str(), MAX_BUFF);
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

void Cgi::execute_child()
{
    close(_pipeIn[1]);
    close(_pipeOut[0]);
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

    execve(_script, argv, _env);
    int error = errno;
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    exit(error);
}