#ifndef WEBSERVEXCEPTION_HPP
# define WEBSERVEXCEPTION_HPP

#include <exception>
#include <string>

class WebservException : public std::exception
{
    private:

        std::string _message;

    public:

        WebservException(std::string message) : _message("WebServer Exception: " + message){}
    

        virtual const char* what() const throw()
        {
            return _message.c_str();
        }

};

#endif