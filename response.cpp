#include "response.hpp"

Response::Response(Client *ptr) : _ptr(ptr)
{
    return ;
}


std::string Response::function()
{
    for (int i = 0; i < MAX_SIZE; i++)
    {
        // if (_ptr->_req_buff[i] != '\0')
        // {
        //     return ;
        // }
        std::cout << "response" << std::endl;
    }
    return NULL;
}