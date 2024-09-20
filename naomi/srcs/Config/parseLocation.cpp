#include "parseLocation.hpp"

parseLocation::parseLocation(void) {

};

parseLocation::~parseLocation(void) {

};


// struct Location {
// 	std::string					path;
// 	std::vector<std::string>	methods;
// 	std::string					root;
// 	std::string					cgi_path;
// 	std::string					upload_dir;
// 	std::vector<std::string>	index; //for if requested resource is directory
// 	std::string					redirect_url;
// 	std::string					max_body_size;
// };

void parseLocation::initKeys(void) {
	_keys["path"] = "";
	_keys["root"] = "";
	_keys["cgi_pass"] = "";
	_keys["upload_dir"] = "";
	_keys["redirect_url"] = "";
	_keys["max_body_size"] = "";
}