#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "webserver.hpp"
# include "response.hpp"
# include "client.hpp"

class Client;

class Request
{

    private:

        Client*          _ptr;

    public:

        Request(Client* ptr);
        ~Request() = default;
        
        bool function(char *buff);

};

#endif