#ifndef EVENT_HANDLER
# define EVENT_HANDLER

#include "webserver.hpp"
#include "request.hpp"
#include "response.hpp"

#define MAX_SIZE 1024

class Event_handler
{
    private:

        Request         _request_handl;
        Response        _response_handl;
        
        char            _buff[MAX_SIZE];
        std::string     _buffer_string;
        size_t          _header_end;
        bool            _done;

        Server*         _server;

    public:

        Event_handler(Server *server);
        ~Event_handler() = default;
        
        void    consume(int event_type);


};

#endif


/*
    Class template? Request and Respond inherit from it
*/