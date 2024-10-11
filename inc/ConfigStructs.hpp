#ifndef CONFIGSTRUCTS_HPP
# define CONFIGSTRUCTS_HPP

#include <map>
#include <vector>
#include <string>

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
	std::string					cgi_pass;
	std::string					upload_dir;
	std::string					redirect_url;
	std::string					max_body_size;
	bool						autoindex;
	std::vector<std::string>	index; //for if requested resource is directory
	std::vector<std::string>	methods;
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
	unsigned int				listen; //port
	std::string					root;
	std::string					cgi_pass;
	std::string					upload_dir;
	std::string					redirect_url;
	std::string					max_body_size;
	bool						autoindex;
	std::vector<std::string>	index; //for if requested resource is directory
	std::vector<std::string>	methods;
	std::map<unsigned int, std::string>	error_pages;
	std::vector<Location>		locations;
};

# define serverMap std::map<std::string, void(serverParser::*)(std::vector<std::string> &args)> //function map for servers
# define locationMap std::map<std::string, void(locationParser::*)(std::vector<std::string> &args)> //function map for locations

#endif