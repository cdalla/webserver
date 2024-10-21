#include "client.hpp"
#include "requestParser.hpp"
#include "responseHandler.hpp"

Client::Client(Server *server): _server(server) {
    _done = false;
    return ;
}

Client::~Client(void) {
    if (request.env != nullptr) {
        for (int i = 0; i < 33; ++i) {
            if (request.env[i] != nullptr) {
                delete[] request.env[i];
            }
        }
        delete[] request.env;
    }
    if (request.script_name != nullptr)
        delete[] request.script_name;
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
        size_t  buffer_size = 1024;
        char    buffer[buffer_size];
        ssize_t bytes_read;
        ssize_t total_bytes_read = 0;
        RequestParser parser(_server->get_config());

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

        this->request = parser.get_parsed_request();
        std::cout << "Read successful: " << total_bytes_read << " bytes" << std::endl;
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