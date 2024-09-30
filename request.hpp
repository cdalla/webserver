#ifndef REQUEST_HPP
# define RESQUEST_HPP

#include "webserver.hpp"

class Request
{

    private:

        Client          *_ptr;

    public:

        Request(Client *ptr);
        ~Request() = default;
        
        int function(char *buff);

};

#endif