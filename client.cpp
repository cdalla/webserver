#include "client.hpp"


Client::Client(Server *server): _server(server), _req_buff(), _resp_string(NULL), _done(false) 
{
    _req_handl = new Request(this);
    _resp_handl = new Response(this);
    return ;
}

bool Client::consume(int event_type)
{
    size_t bytes;

    if (event_type == IN)
    {
        if (!_done)
        {
            bytes = recv(_socket, _req_buff, MAX_SIZE, 0);
            if (bytes == -1)
            {
                //error
                return ;
            }
            _done = _req_handl->function(_req_buff);
            std::memset(_req_buff, 0, MAX_SIZE);
        }
        return (_done);
    }
    else if (event_type == OUT)
    {
        if (!_done)
            return ;
        
        _resp_string = _resp_handl->function();
        bytes = send(_socket, _resp_string.c_str(), _resp_string.size(), 0);
        if (bytes < 0)
        {
            //error
            return ;   
        }
        return (_done);
    }