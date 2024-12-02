#ifndef REQUEST_PARSER_HPP
#define REQUEST_PARSER_HPP
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include "structs.hpp"

class RequestParser
{
private:
    bool _is_header_finish;
    bool _is_first_line;
    std::string _buffer;
    std::string _last_key;
    VirtualServer &_config;
    ssize_t _chunk_size;
    std::string _query_string;
    unsigned int _max_body_size;

    std::string _script_name;
    std::string _protocol;
    Request &finished_request;

    bool _is_chunked;
    std::string _chunked_buffer;
    unsigned long _current_chunk_size;
    bool _is_reading_chunk_size;

    bool set_MetAddProt(void);
    bool parse_protocol(void);
    void set_map(void);
    int set_body(void);
    bool handle_chunked_data(void);

public:
    RequestParser(VirtualServer &config, Request &request);
    ~RequestParser(void);

    bool feed(const char *chunk, ssize_t byte);
    std::string _body;
};

typedef enum e_request_type
{
    GET,
    POST,
    DELETE,
    METHOD_NOT_ALLOW = 405,
    WRONG_PROTOCOL = 505,
    PAYLOAD_TO_LARGE = 413,
    BAD_REQUEST = 400,
} t_request_type;

#endif