#include "event_handler.hpp"

Event_handler::Event_handler(Server *server) : _server(server), _buff(), _buffer_string(NULL), _done(false)
{
    return ;
}

//event can be IN or OUT depends on event triggered by epoll
void Event_handler::consume(int event_type)
{
    if (event_type == IN)
    {

    }
    else if (event_type == OUT)
    {

    }

    switch (event_type)
    {
        case IN:

            break;
        case OUT:
            
            break;
    }
}
