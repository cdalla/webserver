#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "webserver.hpp"

class Response
{
    
    private:

        Client          *_ptr;

    public:

        Response(Client *ptr);
        ~Response() = default;

        std::string function(void);

};

#endif