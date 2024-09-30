#include "request.hpp"

int Request::function()
{
    for (int i = 0; i < MAX_SIZE; i++)
    {
        if (_ptr->_buff[i] != '\0')
        {
            return ;
        }
    }
}