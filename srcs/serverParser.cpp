#include "config.hpp"
#include "serverParser.hpp"
#include "locationParser.hpp"
#include "baseParser.cpp"
#include <iostream>
#include "colours.hpp"

/**
 * TO DO
 * 
 * - home location
 * After server has been parsed, check if there's a "/" location.
 * If not, create a "/" location and copy the following VirtualServer attributes to that location instance:
 * 
 * - de
 */
serverMap	serverParser::_initServerFunctions() {
	serverMap newMap;

	newMap["listen"] = &serverParser::parseListen;
	newMap["cgi_pass"] = &serverParser::parseCgiPass;
	newMap["host"] = &serverParser::parseHost;
	newMap["server_name"] = &serverParser::parseName;
	newMap["root"] = &serverParser::parseRoot;
	newMap["allow_methods"] = &serverParser::parseMethods;
	newMap["index"] = &serverParser::parseIndex;
	newMap["autoindex"] = &serverParser::parseAutoindex;
	newMap["error_page"] = &serverParser::parseErrorPages;
	newMap["max_body_size"] = &serverParser::parseMaxBodySize;

	return newMap;
}

serverMap	serverParser::_serverFunctions = serverParser::_initServerFunctions();


serverParser::serverParser(void) : baseParser<VirtualServer>() {
	context.listen = 0;
	context.autoindex = false; // the default for nginx is also false
	context.server_name = "";
	context.root = "";
	context.cgi_pass = "";
	context.max_body_size = "";
};

serverParser::~serverParser(void) {};

VirtualServer	serverParser::parseServer(std::vector<std::string> &tokens, std::vector<std::string>::iterator &i) {

	std::vector<std::string>	args;
	std::string					directive = "";

	i++; // The token after "server" has to be an opening bracket
	if (i == tokens.end() || *i != "{")
		throw std::runtime_error("Invalid server configuration: missing opening bracket");
	
	i++; // We don't want to parse the opening bracket so increment once more
	
	while (i != tokens.end() && *i != "}") {
		if (_serverFunctions.find(*i) == _serverFunctions.end()) { // token is not in serverMap: it's location or an argument
			//scenario 1: token == "location" -> add result from locationParser to location vector
			if (*i == "location") {
				if (directive != "")
					addDirective(directive, args, *i);
				parseLocation(tokens, i);
				continue ;
			}
			else
				args.push_back(*i);
		}
		else { //token is in serverMap: so it's a directive
			if (directive != "")
				addDirective(directive, args, *i);
			else
				directive = *i;
		}
		i++;
	}
	if (directive != "")
		addDirective(directive, args, "hoho");
	if (i == tokens.end())
		throw std::runtime_error("Invalid server configuration: missing closing bracket");
	checkHomeLocation();
	return context;
}

/* nginx parses listen as
	numbers:numbers -> address:port (port == host)
	numbers			-> address (and 80 will be used for port)
*/

// gonna need to come back to it because i do think we need to build in the functionality of being able to listen on different addresses (so not just the port)
void	serverParser::parseListen(std::vector<std::string> &args) {
	if (args.size() != 1 || context.listen != 0)
		throw ConfigException();
	for (size_t i = 0; i < args[0].length(); i++) {
		if (!isdigit(args[0][i]))
			throw std::runtime_error("Invalid listen directive: port can only contain digits");
	}
	try {
		context.listen = std::stoul(args[0]);
	} catch (std::exception &e) {
		throw std::runtime_error("Invalid listen directive: port not a valid integer");
	}

}

void	serverParser::parseHost(std::vector<std::string> &args) {

}

void	serverParser::parseName(std::vector<std::string> &args) {
	if (args.size() > 1)
		throw std::runtime_error("Invalid server name");
	context.server_name = args.front();
}

void	serverParser::addDirective(std::string &directive, std::vector<std::string> &args, std::string newDirective) {
	if (directive == ""|| !args.size())
		throw ConfigException();

	(this->*serverParser::_serverFunctions[directive])(args);
	args.clear();
	if (newDirective == "location")
		directive = "";
	else
		directive = newDirective;
}

void	serverParser::parseLocation(std::vector<std::string> &tokens, std::vector<std::string>::iterator &i) {
	
	locationParser parser;
	Location	location = parser.parseLocation(tokens, i);

	for (std::vector<Location>::iterator it = context.locations.begin(); it != context.locations.end(); it++) {
		if ((*it).path == location.path)
			return ;

	}
	context.locations.push_back(location);

	// think about what to do when there are multiple locations with the same path
}

void	serverParser::checkHomeLocation(void) {

	//loop through locations, see if location with path "/" exists
	// if yes, return
	// if no, create location with path "/". Move configurations from this server instance to newly created "/" location and push location to back of _locations vector

	for (std::vector<Location>::iterator it = context.locations.begin(); it != context.locations.end(); it++) {
		if ((*it).path == "/") //set empty directives to the default settings
			return ;
	}

	Location	home;
	// copy context directives to home
	// set empty directives to the default settings

	home.root = context.root;
	home.cgi_pass = context.cgi_pass;
	home.upload_dir = context.upload_dir;
	home.redirect_url = context.redirect_url;
	home.max_body_size = context.max_body_size;
	home.autoindex = context.autoindex;
	home.index = context.index;
	home.methods = context.methods;
	home.error_pages = context.error_pages;

	context.locations.push_back(home);
}

std::ostream&   operator<<(std::ostream& out, VirtualServer const &obj) {

	out << PRETTY_RED << "VirtualServer" << RST << std::endl;
	out << "server_name: " << obj.server_name << std::endl;
	out << "listen: " << obj.listen << std::endl;
	out << "root: " << obj.root << std::endl;
	out << "methods: ";
	for (size_t i = 0; i < obj.methods.size(); i++)
		out << obj.methods[i] << " ";
	out << "\ncgi_pass: " << obj.cgi_pass << std::endl;
	out << "upload_dir: " << obj.upload_dir << std::endl;
	out << "max_body_size: " << obj.max_body_size << std::endl;
	out << "autoindex: " << (!obj.autoindex ? "off" : "on") << std::endl;
	out << "index: ";
	for (size_t i = 0; i < obj.index.size(); i++)
		out << obj.index[i] << " ";
	out << "\nerror_pages:" << std::endl;
	for (std::map<unsigned int, std::string>::const_iterator it = obj.error_pages.begin(); it != obj.error_pages.end(); ++it) {
			out << "	" << it->first << " " << it->second << std::endl;
	}
	for(size_t i = 0; i < obj.locations.size(); i++)
		std::cout << obj.locations[i] << std::endl;
	return (out);
}
