#include "client.hpp"

Client::Client(Server *server): _server(server)
{
    _done = false;
    _req_handl = new Request(this);
    _resp_handl = new Response(this);
    return ;
}


Client::~Client()
{
    delete _req_handl;
    delete _resp_handl;
}

/*
    DEPENDING ON EVENT TYPE:
        -IN: fill buffer and pass to request handler
        -OUT: call response handler for data and send data
*/
bool Client::consume(int event_type)
{
    ssize_t bytes;

    if (event_type == IN)
    {
        if (!_done)
        {
            bytes = recv(_socket, _req_buff, MAX_SIZE, 0);
            if (bytes < 0)
            {
                //error
                return false;
            }
            _done = _req_handl->function(_req_buff);
            std::memset(_req_buff, 0, MAX_SIZE);
        }
        return (_done);
    }
    else
    {
        if (!_done)
            return false;
        
        _resp_string = _resp_handl->function();
        bytes = send(_socket, _resp_string.c_str(), _resp_string.size(), 0);
        if (bytes < 0)
        {
            //error
            return false;   
        }
        return (_done);
    }
}