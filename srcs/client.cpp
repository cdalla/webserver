#include "client.hpp"
#include "requestHandler.hpp"
#include "responseHandler.hpp"

Client::Client(Server *server): _server(server) {
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
		requestHandler handler(this);
		request = handler.readRequest();
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
        // _req_handl.readRequest();
        // std::cout << (_req_handl) << std::endl;
        std::cout << request << std::endl;

        return true;
    }
    else
    {
        // if (!_done)
        //     return false;
		responseHandler handler(this);
		response = handler.create(request);
        _resp_string.append(response.statusLine);
        _resp_string.append(response.contentType);
        _resp_string.append(response.contentLength);
        _resp_string.append(response.entityBody);

        bytes = send(_socket, _resp_string.c_str(), _resp_string.size(), 0);
        if (bytes < 0)
        {
            //error
            return false;   
        }
        return (true);
    }
}