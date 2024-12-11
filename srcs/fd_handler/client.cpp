#include "client.hpp"
#include "responseHandler.hpp"
# include "webserver.hpp"



Client::Client(Server *server, Webserver *main, Config* config): server(server), main(main), _config(config)
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
	delete parser;
}

/*
    DEPENDING ON EVENT TYPE:
        -IN: fill buffer and pass to request handler
        -OUT: call response handler for data and send data
*/


void Client::input()
{
    reset_last_activity();
    ssize_t bytes_read;

    if (!_done)
    {
        bytes_read = read(_fd,_req_buff, MAX_BUFF - 1);
        if (bytes_read == -1){
            throw WebservException("Failed read in client");
        }
		else if (bytes_read >= 0)
        	_total_bytes_read += bytes_read;
        _done = parser->feed(_req_buff, bytes_read);
        if (_done)
        {
            main->change_event(_fd);
        }
        std::memset(_req_buff, 0, MAX_BUFF);
        return ;
    }
    main->change_event(_fd);
}

void Client::output()
{
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
    delete handler;
    if (bytes <= 0 || (size_t)bytes == response.size())
    {
        main->removeFd(_fd, CONN, 0);
    }
}