#include "Request.hpp"
#include <string.h>
#include <stdlib.h>


Request::Request(void): _is_header_finish(0), _is_first_line(0),_env(NULL), _max_body_size(0),
    _is_chunked(false), _current_chunk_size(0), _is_reading_chunk_size(true) {}

Request::~Request(void){
	if (_env == NULL)
		return ;
	for (int i = 0; i < 33; i++)
		free(_env[i]);
	free(_env);
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

int Request::set_body(void)
{
	if (_method.compare("GET") == 0)
		return (400);
    
    if (_is_chunked)
        return handle_chunked_data();
    
    _body.append(_buffer);
    _buffer.clear();
    if (_body.size() > _max_body_size)
        return (400);
    return 0;
}

int Request::handle_chunked_data(void)
{
    while (!_buffer.empty())
    {
        if (_is_reading_chunk_size)
        {
            size_t pos = _buffer.find("\r\n");
            if (pos == std::string::npos)
                return 0;
            
            std::string chunk_size_str = _buffer.substr(0, pos);
            _buffer.erase(0, pos + 2);
            
            std::stringstream ss;
            ss << std::hex << chunk_size_str;
            ss >> _current_chunk_size;
            
            if (_current_chunk_size == 0)
            {
                // End of chunked data
                _buffer.clear();
                return 1;
            }
            
            _is_reading_chunk_size = false;
        }
        else
        {
            if (_buffer.size() < _current_chunk_size + 2)
                return 0;
            
            _body.append(_buffer.substr(0, _current_chunk_size));
            _buffer.erase(0, _current_chunk_size + 2);
            _is_reading_chunk_size = true;
        }
    }
    
    return 0;
}

int  Request::parse_protocol(void)
{
	if (_buffer.find_first_of("HTTP/1.1")  == std::string::npos)
		return WRONG_PROTOCOL;
	_protocol = "HTTP/1.1";
	_buffer.erase(0, _buffer.find_first_of('\n') + 1);
	_is_first_line = 1;
	return 0;
}

int  Request::set_MetAddProt(void)
{
	if (_buffer.find_first_of('\n') == std::string::npos)
		return 1;
	_method  = _buffer.substr(0, _buffer.find_first_of(' '));
	if (st_check_method(_method) == METHOD_NOT_ALLOW)
	return 405;
	_buffer.erase(0, _buffer.find_first_of(' ') + 1);
	std::cout << _buffer<< std::endl << std::endl;
	_uri = _buffer.substr(0, _buffer.find_first_of(' '));
	_script_name = _uri;
	_query_string = "";
	_is_cgi = true;
	if (_uri.find('?') != std::string::npos)
	{
		_query_string = _uri.substr(_uri.find('?'), _uri.length());
		_script_name = _uri.substr(0, _uri.find('?'));
	}
	_buffer.erase(0, _buffer.find_first_of(' ') + 1);
	return parse_protocol();
}

void Request::set_map(void)
{
	_is_header_finish = 0;
	// check if there some value in the string
	if (_buffer.find_first_of('\n') == std::string::npos)
		return ;
	std::string value;

	while (_buffer.find_first_of('\n') != 0)
	{
		//checking if part of previus key
		while (_buffer.find_first_of(':') == std::string::npos || _buffer.find_first_of(':') > _buffer.find_first_of('\n'))
		{
			value = _buffer.substr(_buffer.find_first_not_of(" \t"), _buffer.find_first_of("\r\n"));
				_buffer.erase(0 ,_buffer.find_first_of('\n') + 1);
				if((_header[ _last_key]).find_last_of(',') != (_header[ _last_key]).length())
				_header[ _last_key].append(", ");
			_header[ _last_key].append(value);
		}
		if (_buffer.find_first_of('\n') != 0)
			break ;
		_last_key = _buffer.substr(0, _buffer.find_first_of(':'));
		_buffer.erase(0,_buffer.find_first_of(':') + 2);
		if (_buffer.find_first_of("\r\n") != std::string::npos)
			value = _buffer.substr(0, _buffer.find_first_of("\r\n"));
		else
			value = _buffer.substr(_buffer.find_first_not_of(" \t"), _buffer.find_first_of('\n'));
		_buffer.erase(0,_buffer.find_first_of('\n') + 1);
		_header[ _last_key] =  value;
		if (_buffer.find_first_of('\n') == std::string::npos)
			return ;
	}
	_is_header_finish = 1;
	if (_buffer.find_first_of("\r\n") == std::string::npos)
		_buffer.erase(0, 1);
	else
		_buffer.erase(0, 2);
    
    if (_header.find("Transfer-Encoding") != _header.end() &&
        _header["Transfer-Encoding"].find("chunked") != std::string::npos)
    {
        _is_chunked = true;
    }
}

int Request::feed(const char *chunk)
{
	_buffer.append(chunk);
	if (!_is_first_line)
    {
		int result = set_MetAddProt();
        if (result)
            return result;
    }
	if (_is_first_line && !_is_header_finish && !result)
		set_map();
        if (!_is_header_finish)
            return 0;
	if (_is_header_finish && _max_body_size == 0)
	{   
		_max_body_size = 66666666;
        if (!_is_chunked)
        { 
            if (_header.find("Content-Length") == _header.end() || stoi(_header["Content-Length"]) < 0 )
                return 401;
            else if (_max_body_size > unsigned(stoi(_header["Content-Length"])))
                _max_body_size = stoi(_header["Content-Length"]); 
        }
    }
	if (_is_first_line && _is_header_finish)
    {
        int body_result = set_body();
        if (body_result != 0)
            return body_result;

        if (_is_chunked)
        {
            if (_current_chunk_size == 0 && _is_reading_chunk_size)
            {
                create_env();
                return 1;
            }
        }
        else
        {
            if (_header.find("Content-Length") != _header.end())
            {
                if (_body.size() == (size_t)stoi(_header["Content-Length"]))
                {
                    create_env();
                    return 1;
                }
            }
            else
            {
                // If no Content-Length and not chunked, assume the request is complete
                create_env();
                return 1;
            }
        }
    }

    return 0;
}

std::string Request::get_uri(void) const
{
	return _uri;
}

std::string Request::get_info(std::string key) const
{
	return _header.find(key)->second;
}

bool  Request::is_info_present(std::string key) const
{
	if ( _header.find(key) == _header.end())
		return false;
	return true;
}

int Request::get_method(void) const
{
	if (_method.compare("GET") == 0)
		return GET;
	if (_method.compare("POST") == 0)
		return POST;
	return DELETE;
}

std::string Request::get_protocol(void) const
{
	return _protocol;
}

const char *Request::get_body(void) const
{
	return _body.c_str();
}


size_t Request::get_bodySize(void) const
{
	return _body.length();
}

static char * joing_string(const char *str1, const char *str2)
{
	size_t size = strlen(str1) + strlen(str2);
	char *string = (char *)malloc((size + 1) * sizeof(char));
	strcpy(string, str1);
	strcat(string, str2);
	string[size] = '\0';
	return (string);
}

void Request::create_env(void)
{
	_env = (char **)malloc(sizeof(char *) * 33);
	_env[32] = NULL;
	_env[0] = joing_string("COMSPEC=","");
     //TODO: coming from config
	_env[1] = joing_string("DOCUMENT_ROOT=","");
	_env[2] = joing_string("GATEWAY_INTERFACE=","");
	_env[3] = joing_string("HOME=","");
	_env[4] = joing_string("HTTP_ACCEPT=","");
	_env[5] = joing_string("HTTP_ACCEPT_CHARSET=","");
	_env[6] = joing_string("HTTP_ACCEPT_ENCODING=","");
	_env[7] = joing_string("HTTP_ACCEPT_LANGUAGE=","");
	_env[8] = joing_string("HTTP_CONNECTION=","");
	_env[9] = joing_string("HTTP_HOST=","");
	_env[10] = joing_string("HTTP_USER_AGENT=","");
	_env[11] = joing_string("PATH=","");
	_env[12] = joing_string("PATHEXT=","");
	_env[13] = joing_string("PATH_INFO=","");
	_env[14] = joing_string("PATH_TRANSLATED=","");
	_env[15] = joing_string("QUERY_STRING=",_query_string.c_str());
	_env[16] = joing_string("REMOTE_ADDR=","");
	_env[17] = joing_string("REMOTE_PORT=","");
	_env[18] = joing_string("REQUEST_METHOD=", _method.c_str());
	_env[19] = joing_string("REQUEST_URI=", _uri.c_str());
    // TODO: coming from config
	_env[20] = joing_string("SCRIPT_FILENAME=","DOCUMENT_ROOT+filename");
	_env[21] = joing_string("SCRIPT_NAME=",_script_name.c_str());
	_env[22] = joing_string("SERVER_ADDR=","");
	_env[23] = joing_string("SERVER_ADMIN=","");
	_env[24] = joing_string("SERVER_NAME=","");
	_env[25] = joing_string("SERVER_PORT=","");
	_env[26] = strdup("SERVER_PROTOCOL=HTTP/1.1");
	_env[27] = joing_string("SERVER_SIGNATURE=","");
	_env[28] = joing_string("SERVER_SOFTWARE=","");
	_env[29] = joing_string("SYSTEMROOT=","");
	_env[30] = joing_string("TERM=","");
	_env[31] = joing_string("WINDIR=","");
}

char *const *Request::get_env(void) const
{	
	return (_env);
}

bool Request::is_cgi(void) const
{
	return _is_cgi;
}

const char *Request::get_script_addr(void) const
{
	return _script_name.c_str();
}

int	Request::is_complete_request(void)
{
	if (_is_header_finish && _is_first_line)
	{
		create_env();
		return 0;
	}
	return 400;
}

std::string Request::get_header_value(std::string key) const
{
    if (_header.find(key) == _header.end())
        return "";
    return _header.find(key)->second;
}