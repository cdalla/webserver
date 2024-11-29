#include "client.hpp"
#include "requestParser.hpp"
#include "responseHandler.hpp"



Client::Client(Server *server, Webserver *main): server(server), main(main) 
{
    _done = false;
    file_content = "";
    reset_last_activity();
    return ;
}


Client::~Client(void) {
}

/*
    DEPENDING ON EVENT TYPE:
        -IN: fill buffer and pass to request handler
        -OUT: call response handler for data and send data
*/


void Client::input()
{
    //std::cout << "handling input event on fd: " << this->get_fd() << std::endl;
    reset_last_activity();
    size_t buffer_size = 1024;
    char buffer[buffer_size];
    ssize_t bytes_read;
    ssize_t total_bytes_read = 0;
    RequestParser* parser = new RequestParser(server->get_config(), this->request);
    std::memset(buffer, 0, buffer_size);
    while ((bytes_read = read(_fd, buffer, buffer_size - 1)) > 0) 
    {
        //std::cout << "buffer read in client: \n" << buffer << std::endl;
        total_bytes_read += bytes_read;
        if (parser->feed(buffer))
            break;
        std::memset(buffer, 0, buffer_size);
    }
    delete parser;
    if (bytes_read == -1 || total_bytes_read == 0)
        throw WebservException("Failed read in client: " + std::string(strerror(errno)));
    //std::cout << "Read successful: " << total_bytes_read << " bytes" << std::endl; 
    main->change_event(_fd);
}

void Client::output()
{
    //std::cout << "handling output event on fd: " << this->get_fd() << std::endl;
    reset_last_activity();
	if (status == "FILE" || status == "CGI")
		return;
    responseHandler* handler = new responseHandler(this);
	if (status == "FILE" || status == "CGI")
    {
        delete handler;
        return;
    }
    std::string response = handler->get();
    ssize_t bytes = send(_fd, response.c_str(), response.size(), 0);
    //std::cout << "Response sent: \n" << response << std::endl;
    //std::cout << "bytes sent: " << bytes << std::endl;
    delete handler;
    if (bytes <= 0 || (size_t)bytes == response.size())
    {
        main->removeFd(_fd, CONN, 0);  
    }
    //what if chunked response
}