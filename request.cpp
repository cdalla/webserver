#include "request.hpp"

Request::Request(Client *ptr) : _ptr(ptr)
{
    return ;
}

bool Request::function(char *buff)
{
    for (int i = 0; i < MAX_SIZE; i++)
    {
        if (buff[i] != '\0')
        {
            return false;
        }
    }
    return false;
}