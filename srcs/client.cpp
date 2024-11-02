#include "client.hpp"
#include "requestParser.hpp"
#include "responseHandler.hpp"


Client::Client(Server *server): _server(server) {
    _done = false;
    return ;
}

Client::~Client(void) {
    if (this->get_socket() != -1) {
        close(this->get_socket());
    }
}

/*
    DEPENDING ON EVENT TYPE:
        -IN: fill buffer and pass to request handler
        -OUT: call response handler for data and send data
*/
bool Client::consume(int event_type)
{
    ssize_t bytes;


    std::cout << "handling event on fd: " << this->get_socket() << std::endl;
    if (event_type == IN)
    {
        size_t  buffer_size = 1024;
        char    buffer[buffer_size];
        ssize_t bytes_read;
        ssize_t total_bytes_read = 0;
        RequestParser parser(server->get_config(), this->request);

        int client_socket = this->get_socket();
        std::memset(buffer, 0, buffer_size);
        while ((bytes_read = read(client_socket, buffer, buffer_size - 1)) > 0) {
            total_bytes_read += bytes_read;
            if (parser.feed(buffer))
                break ;
            std::memset(buffer, 0, buffer_size);
        }
        if (bytes_read == -1 || total_bytes_read == 0) {
            std::cerr << "Read failed" << std::endl;
            return false;
        }
        std::cout << "Read successful: " << total_bytes_read << " bytes" << std::endl;
        return true;
    }
    else if (event_type == OUT)
    {
        // std::cout << "OUT EVENT" << std::endl;

        // if (!_done)
        //     return false;
		responseHandler handler(this);
        std::string response = handler.get();

        bytes = send(_socket, response.c_str(), response.size(), 0);
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