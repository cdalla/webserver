#include "requestParser.hpp"
#include <string.h>
#include <stdlib.h>
#include <sstream> 

RequestParser::RequestParser(VirtualServer config, Request &request): _is_header_finish(0), _is_first_line(0),
    _is_chunked(false), _current_chunk_size(0), _is_reading_chunk_size(true), _config(config), finished_request(request) {
	finished_request.error = 0;
	_max_body_size = config.max_body_size.empty() ? 0 : std::stoi(config.max_body_size);
	finished_request.script_name = NULL;
	finished_request.env = new char*[33];
    for (int i = 0; i < 32; ++i) {
        finished_request.env[i] = NULL;
    }
    finished_request.env[32] = NULL;

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
    
    finished_request.body.append(_buffer);
    _buffer.clear();
    if (finished_request.body.size() > _max_body_size)
        return (PAYLOAD_TO_LARGE);
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

bool  RequestParser::parse_protocol(void)
{
	if (_buffer.find_first_of("HTTP/1.1")  == std::string::npos){
		finished_request.error = WRONG_PROTOCOL;
		return true;
	}
	_protocol = "HTTP/1.1";
	// std::cout << "&protocol " << _protocol << std::endl;
	_buffer.erase(0, _buffer.find_first_of('\n') + 1);
	_is_first_line = 1;
	return false;
}

bool  RequestParser::set_MetAddProt(void)
{
	if (_buffer.find_first_of('\n') == std::string::npos)
	{
		// std::cout << "no new line" << std::endl;
		return true;
	}
	finished_request.method  = _buffer.substr(0, _buffer.find_first_of(' '));
	// std::cout << "&method " << finished_request.method << std::endl;
	if (st_check_method(finished_request.method) == METHOD_NOT_ALLOW){
		finished_request.error = METHOD_NOT_ALLOW;
		// std::cout << "method not allow" << std::endl;
		return true;
	}
	_buffer.erase(0, _buffer.find_first_of(' ') + 1);
	finished_request.uri = _buffer.substr(0, _buffer.find_first_of(' '));
	// std::cout << "&uri " << finished_request.uri << std::endl;
	_script_name = finished_request.uri;
	_query_string = "";
	finished_request.is_cgi = true;
	if (finished_request.uri.find('?') != std::string::npos)
	{
		_query_string = finished_request.uri.substr(finished_request.uri.find('?'), finished_request.uri.length());
		_script_name = finished_request.uri.substr(0,finished_request.uri.find('?'));
	}
	finished_request.script_name = new char[_script_name.size() + 1];
    strcpy(finished_request.script_name, _script_name.c_str());
	_buffer.erase(0, _buffer.find_first_of(' ') + 1);
	return parse_protocol();
}

void RequestParser::set_map(void)
{
	_is_header_finish = 0;
	// check if there some value in the string
	if (_buffer.find_first_of('\n') == std::string::npos)
	{
		std::cout << "no finished header" << std::endl;
		return ;
	}
	std::string value;
	while (_buffer.size() && _buffer.find_first_of('\n') != 0)
	{
		if (_buffer.find_first_of("\r\n\r\n") == 0 || _buffer.find_first_of("\n\n") == 0){
			break;
		}
		//checking if part of previus key
		while (_buffer.find_first_of(':') == std::string::npos || _buffer.find_first_of(':') > _buffer.find_first_of('\n'))
		{
			value = _buffer.substr(_buffer.find_first_not_of(" \t"), _buffer.find_first_of("\r\n"));
			// std::cout << "1value " << value << std::endl;
			_buffer.erase(0 ,_buffer.find_first_of('\n') + 1);
			if((finished_request.headers[ _last_key]).find_last_of(',') != (finished_request.headers[ _last_key]).length())
				finished_request.headers[ _last_key].append(", ");
			finished_request.headers[ _last_key].append(value);
		}
		_last_key = _buffer.substr(0, _buffer.find_first_of(':'));
		// std::cout << "key " << _last_key << std::endl;
		_buffer.erase(0,_buffer.find_first_of(':') + 2);
		if (_buffer.find_first_of("\r\n") != std::string::npos)
			value = _buffer.substr(0, _buffer.find_first_of("\r\n"));
		else
			value = _buffer.substr(_buffer.find_first_not_of(" \t"), _buffer.find_first_of('\n'));
		// std::cout << "value " << value << std::endl;
		_buffer.erase(0,_buffer.find_first_of('\n') + 1);
		finished_request.headers[ _last_key] =  value;
		if (_buffer.find_first_of('\n') == std::string::npos)
			return ;
	}
	_is_header_finish = 1;
	if (_buffer.find_first_of("\r\n\r\n") == std::string::npos)
		_buffer.erase(0, 2);
	else
		_buffer.erase(0, 4);
    if (finished_request.headers.find("Transfer-Encoding") != finished_request.headers.end() &&
        finished_request.headers["Transfer-Encoding"].find("chunked") != std::string::npos)
    {
        _is_chunked = true;
    }
}

bool RequestParser::feed(const char *chunk)
{
	_buffer.append(chunk);
	// std::cout << "buffer " << _buffer << std::endl;
	if (!_is_first_line)
    {
		int result = set_MetAddProt();
        if (result)
            return result;
    }
	if (_is_first_line && !_is_header_finish){
		// std::cout << "working on the headers" << std::endl;
		set_map();
        if (!_is_header_finish)
            return 0;
	}
	if (_is_header_finish)
	{   
		// std::cout << "headers finished" << std::endl;
        if (!_is_chunked)
        { 
            if (finished_request.headers.find("Content-Length") == finished_request.headers.end() || stoi(finished_request.headers["Content-Length"]) < 0 )
                return 401;
            else if (_max_body_size > unsigned(stoi(finished_request.headers["Content-Length"])))
                _max_body_size = stoi(finished_request.headers["Content-Length"]); 
        }
    }
	if (_is_first_line && _is_header_finish)
    {	
		std::cout << "working on the body" << std::endl;
        int body_result = set_body();
        if (body_result != 0)
			finished_request.error = body_result;
            return true;

        if (_is_chunked)
        {
            if (_current_chunk_size == 0 && _is_reading_chunk_size)
            {
                create_env();
                return true;
            }
        }
        else
        {
            if (finished_request.headers.find("Content-Length") != finished_request.headers.end())
            {
                if (_body.size() == (size_t)stoi(finished_request.headers["Content-Length"]))
                {
                    create_env();
                    return true;
                }
            }
            else
            {
                // If no Content-Length and not chunked, assume the request is complete
                create_env();
                return true;
            }
        }
    }

    return false;
}


static char * joing_string(const char *str1, const char *str2)
{
	size_t size = strlen(str1) + strlen(str2);
	char* string = new char[size + 1];
	strcpy(string, str1);
	strcat(string, str2);
	string[size] = '\0';
	return (string);
}

void RequestParser::create_env(void)
{
	finished_request.env[0] = joing_string("COMSPEC=","");
	finished_request.env[1] = joing_string("DOCUMENT_ROOT=",_config.root.c_str());
	finished_request.env[2] = joing_string("GATEWAY_INTERFACE=","");
	finished_request.env[3] = joing_string("HOME=","");
	finished_request.env[4] = joing_string("HTTP_ACCEPT=","");
	finished_request.env[5] = joing_string("HTTP_ACCEPT_CHARSET=","");
	finished_request.env[6] = joing_string("HTTP_ACCEPT_ENCODING=","");
	finished_request.env[7] = joing_string("HTTP_ACCEPT_LANGUAGE=","");
	finished_request.env[8] = joing_string("HTTP_CONNECTION=","");
	finished_request.env[9] = joing_string("HTTP_HOST=","");
	finished_request.env[10] = joing_string("HTTP_USER_AGENT=","");
	finished_request.env[11] = joing_string("PATH=","");
	finished_request.env[12] = joing_string("PATHEXT=","");
	finished_request.env[13] = joing_string("PATH_INFO=","");
	finished_request.env[14] = joing_string("PATH_TRANSLATED=","");
	finished_request.env[15] = joing_string("QUERY_STRING=",_query_string.c_str());
	finished_request.env[16] = joing_string("REMOTE_ADDR=","");
	finished_request.env[17] = joing_string("REMOTE_PORT=","");
	finished_request.env[18] = joing_string("REQUEST_METHOD=", finished_request.method.c_str());
	finished_request.env[19] = joing_string("REQUEST_URI=", finished_request.uri.c_str());
	finished_request.env[20] = joing_string("SCRIPT_FILENAME=",(_config.root + _script_name).c_str());
	finished_request.env[21] = joing_string("SCRIPT_NAME=",_script_name.c_str());
	finished_request.env[22] = joing_string("SERVER_ADDR=","");
	finished_request.env[23] = joing_string("SERVER_ADMIN=","");
	finished_request.env[24] = joing_string("SERVER_NAME=",_config.server_name.c_str());
	finished_request.env[25] = joing_string("SERVER_PORT=",std::to_string(_config.listen).c_str());
	finished_request.env[26] = strdup("SERVER_PROTOCOL=HTTP/1.1");
	finished_request.env[27] = joing_string("SERVER_SIGNATURE=","");
	finished_request.env[28] = joing_string("SERVER_SOFTWARE=","");
	finished_request.env[29] = joing_string("SYSTEMROOT=","");
	finished_request.env[30] = joing_string("TERM=","");
	finished_request.env[31] = joing_string("WINDIR=","");
}
