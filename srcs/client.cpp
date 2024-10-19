#include "client.hpp"

Client::Client(Server *server): _server(server), _req_handl(this), _resp_handl(this)
{
    _done = false;
    return ;
}


Client::~Client(void) {}

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
        // if (!_done)
        // {
        //     bytes = recv(_socket, _req_buff, 10, 0);
        //     if (bytes < 0)
        //     {
        //         //error
        //         return false;
        //     }
        //     _done = _req_handl->function(_req_buff);
        //     std::memset(_req_buff, 0, MAX_SIZE);
        // }
        // return (_done);
        _req_handl.readRequest();
        std::cout << (_req_handl) << std::endl;
        return true;
    }
    else
    {
        // if (!_done)
        //     return false;
        
        _resp_handl.create((_req_handl));
        _resp_string.append(_resp_handl.statusLine);
        _resp_string.append(_resp_handl.contentType);
        _resp_string.append(_resp_handl.contentLength);
        _resp_string.append(_resp_handl.entityBody);

        bytes = send(_socket, _resp_string.c_str(), _resp_string.size(), 0);
        if (bytes < 0)
        {
            //error
            return false;   
        }
        return (true);
    }
}