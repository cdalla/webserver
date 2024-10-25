#include "client.hpp"
#include "requestHandler.hpp"
#include "responseHandler.hpp"

Client::Client(Server *server): _server(server) {
    _done = false;
    reset_last_activity();
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
    std::cout << "handling event on fd: " << this->get_socket() << std::endl;
    reset_last_activity();

    ssize_t bytes;


    if (event_type == IN)
    {
        // std::cout << "IN EVENT" << std::endl;
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
       // std::cout << request << std::endl;

        return true;
    }
    else if (event_type == OUT)
    {
        // std::cout << "OUT EVENT" << std::endl;

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
    else
    {
        std::cout << "EVENT PASSED TO CLIENT ERROR" << std::endl;
        return true;
    }
}


void Client::reset_last_activity()
{
    _last_activity = std::chrono::steady_clock::now();
}

bool Client::has_timeout()
{
    auto now = std::chrono::steady_clock::now();
    auto difference = std::chrono::duration_cast<std::chrono::seconds>(now - _last_activity);
    if (difference.count() > TIMEOUT)
        return true;
    return false; 
}