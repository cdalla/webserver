#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP
# include <string>
# include <vector>
# include "Request.hpp"

class CgiHandler
{
    private:
        int _fd_in[2];
        int _fd_out[2];
        int _status_code;
        std::string _response;
        std::string _status_mess;
        int initialize_pipe(void);
        void child_exe(const char *script, char * const *env);
        void parent_exe(const char *body, pid_t pid);

    public:
        CgiHandler(const char *script, char * const *env, const char *body);
        ~CgiHandler(void);
        int get_status_code (void) const;
        std::string get_status_mess (void) const;
        std::string get_response (void) const;

};


#endif
