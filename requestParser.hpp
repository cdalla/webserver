#ifndef REQUEST_PARSER_HPP
# define REQUEST_PARSER_HPP
# include <string>
# include <iostream>
# include <fstream>
# include <map>

class Request
{
    private:
        bool    _is_header_finish;
        bool    _is_first_line;
        std::string _buffer;
        std::string _last_key;

        std::string _method;
        bool        _is_cgi;
        char        **_env;

        std::map<std::string, std::string>  _header;

        std::string     _query_string;
        unsigned int    _max_body_size;
        std::string     _uri;
        std::string     _script_name;
        std::string     _protocol;
        std::string     _body;

        bool            _is_chunked;
        std::string     _chunked_buffer;
        unsigned long   _current_chunk_size;
        bool            _is_reading_chunk_size;

        int     set_MetAddProt(void);
        int     parse_protocol(void);
        void    set_map(void);
        int     set_body(void);
        int     handle_chunked_data(void);
        
        size_t      get_bodySize(void) const;
        void        create_env(void);
        bool        is_info_present(std::string key) const;
        std::string get_info(std::string key) const;
        std::string get_protocol(void) const;
        
    public:
        Request(void);
        ~Request(void);

        int         feed(const char* chunk);
        std::string get_uri(void) const;
        int         get_method(void) const;
        std::string get_header_value( std::string key) const;
        const char  *get_body(void) const;
        char *const *get_env(void) const;
        bool        is_cgi(void) const;
        const char  *get_script_addr(void) const;
};

typedef enum e_request_type
{
    GET,
    POST,
    DELETE,
    METHOD_NOT_ALLOW = 405,
    WRONG_PROTOCOL = 505,
} t_request_type;

#endif