#include "client.hpp"
#include "responseHandler.hpp"
# include "webserver.hpp"



Client::Client(Server *server, Webserver *main): server(server), main(main) 
{
    _done = false;
    _total_bytes_read = 0;
    std::memset(_req_buff, 0, MAX_BUFF);
    file_content = "";
    parser = new RequestParser(server->get_config(), this->request);
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
    ssize_t bytes_read;

    // bytes_read = read(_fd, buffer, buffer_size - 1);
    // total_bytes_read += bytes_read;
    if (!_done)
    {
        bytes_read = read(_fd,_req_buff, MAX_BUFF - 1);
        //std::cout << "buffer read in client: \n" << buffer << std::endl;
        if (bytes_read == -1){
            throw WebservException("Failed read in client: " + std::string(strerror(errno)));
        }
        _total_bytes_read += bytes_read;
        _done = parser->feed(_req_buff, bytes_read);
        std::memset(_req_buff, 0, MAX_BUFF);
        return ;
    }
    delete parser;
    main->change_event(_fd);
    //std::cout << "Read successful: " << total_bytes_read << " bytes" << std::endl;
    //std::cout << "Read body: " << this-> request.body.size() << std::endl; 

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