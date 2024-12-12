#ifndef RESPONSEHANDLER_HPP
#define RESPONSEHANDLER_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include "client.hpp"
#include "defines.hpp"
#include "structs.hpp"

class Client;

class responseHandler
{

private:
	Client *_client;
	VirtualServer *_config;
	Webserver *_main;
	std::string _file;
	std::string _content_type;
	std::string _body;
	std::string _root;
	std::string _response;
	std::string _upload_dir;
	std::string _redirect_url;
	ssize_t _max_body_size;
	char **_env;
	std::map<unsigned int, std::string> _error_pages;
	std::vector<std::string> _cgi_ext;
	std::vector<std::string> _index;
	bool _autoindex;

	std::string _getStatusMessage(int error);
	void _determineType(std::string path);
	void _createErrorPage(int error);
	void _handleError(int error);
	void _handleErrorPage(int error, std::string error_page);
	void _handleDefaultError(int error);
	void _handlePage(std::string path);
	void _handleDirectory(std::string path);
	void _handleCGI(std::string path);
	void _createEnv(void);
	void _createResponse(void);
	void _locationHandler(std::string path);
	void _handleDirRequest(std::string path);
	void _handleRedirect(std::string path);
	void _setRightConfig(void);

public:
	responseHandler(Client *ptr);
	~responseHandler(void);

	std::string get(void);
};

#endif
