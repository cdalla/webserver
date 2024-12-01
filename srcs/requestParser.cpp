#include "requestParser.hpp"
#include <string.h>
#include <stdlib.h>
#include <sstream> 
#include "colours.hpp"

RequestParser::RequestParser(VirtualServer &config, Request &request): _is_header_finish(0), _is_first_line(0),
    _is_chunked(false), _current_chunk_size(0), _is_reading_chunk_size(true), _config(config), finished_request(request) {
	finished_request.error = 0;
	_max_body_size = config.max_body_size;
	if (_max_body_size < 0)
		_max_body_size = 1000000;
}

RequestParser::~RequestParser(void){
}


int st_check_method(std::string method)
{
	if (method.compare("GET") == 0)
		return GET;
	if (method.compare("POST") == 0)
		return POST;
	if (method.compare("DELETE") == 0)
		return DELETE;
	return METHOD_NOT_ALLOW;
}

int RequestParser::set_body(void)
{
	if (finished_request.method.compare("GET") == 0)
		return (BAD_REQUEST);
    
    if (_is_chunked)
        return handle_chunked_data();
    std::cout << "inside parser " << _buffer << "  \nbuffer size " << _buffer.size() << std::endl; 
    finished_request.body.append(_buffer);

    _buffer.clear();
    if (finished_request.body.size() > _max_body_size)
        return (PAYLOAD_TO_LARGE);
	std::cout << "finished_request.body : \n" << finished_request.body << std::endl;
    return 0;
}

bool RequestParser::handle_chunked_data(void)
{
    while (!_buffer.empty())
    {
        if (_is_reading_chunk_size)
        {
            size_t pos = _buffer.find("\r\n");
            if (pos == std::string::npos)
                return false;
            
            std::string chunk_size_str = _buffer.substr(0, pos);
            _buffer.erase(0, pos + 2);
            
            std::stringstream ss;
            ss << std::hex << chunk_size_str;
            ss >> _current_chunk_size;
            
            if (_current_chunk_size == 0)
            {
                // End of chunked data
                _buffer.clear();
                return true;
            }
            
            _is_reading_chunk_size = false;
        }
        else
        {
            if (_buffer.size() < _current_chunk_size + 2)
                return false;

			// Check if adding this chunk would exceed the max body size
            if (finished_request.body.size() + _current_chunk_size > _max_body_size)
            {
                finished_request.error = PAYLOAD_TO_LARGE;
                return true;
            }
            
            _body.append(_buffer.substr(0, _current_chunk_size));
            _buffer.erase(0, _current_chunk_size + 2);
            _is_reading_chunk_size = true;
        }
    }
    
    return false;
}

void  RequestParser::parse_protocol(void)
{
	if (_buffer.find_first_of("HTTP/1.1")  == std::string::npos)
		finished_request.error = WRONG_PROTOCOL;

	_protocol = "HTTP/1.1";
	// std::cout << "&protocol " << _protocol << std::endl;
	_buffer.erase(0, _buffer.find_first_of('\n') + 1);
}

void  RequestParser::set_MetAddProt(void)
{

	finished_request.method  = _buffer.substr(0, _buffer.find_first_of(' '));
	// std::cout << "&method " << finished_request.method << std::endl;
	if (st_check_method(finished_request.method) == METHOD_NOT_ALLOW){
		finished_request.error = METHOD_NOT_ALLOW;
	}
	_buffer.erase(0, _buffer.find_first_of(' ') + 1);
	finished_request.uri = _buffer.substr(0, _buffer.find_first_of(' '));
	// std::cout << "&uri " << finished_request.uri << std::endl;
	_script_name = finished_request.uri;
	_query_string = "";
	if (finished_request.uri.find('?') != std::string::npos)
	{
		_query_string = finished_request.uri.substr(finished_request.uri.find('?'), finished_request.uri.length());
		_script_name = finished_request.uri.substr(0,finished_request.uri.find('?'));
	}
	finished_request.script_name = _script_name;

	_buffer.erase(0, _buffer.find_first_of(' ') + 1);
	parse_protocol();
}

void RequestParser::set_map(void)
{
	std::string value;
	while (_buffer.size() && _buffer.find_first_of('\n') != 0)
	{
		if (_buffer.find_first_of("\r\n\r\n") == 0 || _buffer.find_first_of("\n\n") == 0){
			break;
		}

		_last_key = _buffer.substr(0, _buffer.find_first_of(':'));
		_buffer.erase(0,_buffer.find_first_of(':') + 2);
		if (_buffer.find_first_of("\r\n") != std::string::npos)
			value = _buffer.substr(0, _buffer.find_first_of("\r\n"));
		else
			value = _buffer.substr(_buffer.find_first_not_of(" \t"), _buffer.find_first_of('\n'));
		// std::cout << "value " << value << std::endl;
		_buffer.erase(0,_buffer.find_first_of('\n') + 1);
		finished_request.headers[ _last_key] =  value;
	}
	_is_header_finish = true;
	if (_buffer.find_first_of("\r\n\r\n") == std::string::npos)
		_buffer.erase(0, 2);
	else
		_buffer.erase(0, 4);
    if (finished_request.headers.find("Transfer-Encoding") != finished_request.headers.end() &&
        finished_request.headers["Transfer-Encoding"].find("chunked") != std::string::npos)
        _is_chunked = true;
}

bool RequestParser::feed(const char *chunk, ssize_t size)
{
	_buffer.append(chunk, size);
    
    if (!_is_header_finish) {
        if (_buffer.find("\r\n\r\n") == std::string::npos) {
            return false;
        }
		set_MetAddProt();
		set_map();
		if (finished_request.headers.find("Content-Length") != finished_request.headers.end()){
			if (stoi(finished_request.headers["Content-Length"]) == 0 )
				return true;
			if (stoi(finished_request.headers["Content-Length"]) > _max_body_size){
				finished_request.error = PAYLOAD_TO_LARGE;
				return true;
			}
			if (stoi(finished_request.headers["Content-Length"]) < _max_body_size)
				_max_body_size = stoi(finished_request.headers["Content-Length"]);
		}

    }

	//std::cout << "working on the body" << std::endl;
    int body_result = set_body();
	if (body_result != 0){
		finished_request.error = body_result;
		return true;
	}
	if (_is_chunked)
	{
		if (_current_chunk_size == 0 && _is_reading_chunk_size)
			return true;
	}
	else if (_body.size() == (size_t)stoi(finished_request.headers["Content-Length"]))
		return true;

    return false;
}



