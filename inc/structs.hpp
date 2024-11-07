#ifndef STRUCTS_HPP
# define STRUCTS_HPP

#include <map>
#include <vector>
#include <string>
#include <list>

class locationParser;

/**
 * @param path string
 * @param root string
 * @param cgi_pass string
 * @param upload_dir string
 * @param redirect_url string
 * @param max_body_size string
 * @param autoindex bool
 * @param index vector<string>
 * @param methods vector<string>
 * @param error_pages vector<uint, string>
 */
struct Location {
	std::string					path;
	std::string					root;
	std::list<std::string>		cgi_ext;
	std::string					upload_dir;
	std::string					redirect_url;
	std::string					max_body_size;
	bool						autoindex;
	std::list<std::string>		index; //for if requested resource is directory
	std::list<std::string>		methods;
	std::map<unsigned int, std::string>	error_pages;
};

/**
 * @param server_name string
 * @param listen unsigned int
 * @param root string
 * @param cgi_pass string
 * @param upload_dir string
 * @param redirect_url string
 * @param max_body_size string
 * @param autoindex bool
 * @param index vector<string>
 * @param methods vector<string>
 * @param error_pages vector<uint, string>
 * @param locations vector<Location>
 */
struct VirtualServer
{
	std::string					server_name;
	std::map<std::string, unsigned int>	listendirective;
	unsigned int				listen; //port
	std::string					root;
	std::vector<std::string>	cgi_ext;
	std::string					upload_dir;
	std::string					redirect_url; //to do: implemnt defining HTTP redirection in config file
	std::string					max_body_size;
	bool						autoindex;
	std::vector<std::string>	index; //for if requested resource is directory
	std::vector<std::string>	methods;
	std::map<unsigned int, std::string>	error_pages;
	std::vector<Location>		locations;
	std::map<std::string, Location> locationMap;
};

# define serverMap std::map<std::string, void(serverParser::*)(std::vector<std::string> &args)> //function map for servers
# define locationMap std::map<std::string, void(locationParser::*)(std::vector<std::string> &args)> //function map for locations

struct Request {

	bool		exists;
	int			error; // if not error 0

	std::string		method;
	std::string		resource;
	std::string		extension;
	std::string		uri;
	std::string		_path;
	std::string		body;
	std::string 	query_string;
	std::map<std::string, std::string>	headers;
	std::string			script_name;

};

struct Response {
	std::string	statusLine;
	std::string	contentType;
	std::string	contentLength;
	std::string	entityBody;
};

#endif