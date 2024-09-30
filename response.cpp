#include "response.hpp"

void Response::function()
{
    for (int i = 0; i < MAX_SIZE; i++)
    {
        if (_ptr->_buff[i] != '\0')
        {
            return ;
        }
    }
}