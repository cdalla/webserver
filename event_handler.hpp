#ifndef EVENT_HANDLER
# define EVENT_HANDLER

class Event_handler
{
    private:

        Request         request_handl;
        Response        response_handl;
        
        char            buff[MAX_SIZE];
        std::string     buffer_string;
        size_t          header_end;
        bool            done;

        Server*          server;

    public:

        Event_handler(Server *server);
        ~Event_handler() = default;


};

#endif