#include "config.hpp"
#include "serverParser.hpp"
#include "locationParser.hpp"
#include "baseParser.cpp"
#include <iostream>
#include "colours.hpp"

serverMap	serverParser::_initServerFunctions() {
	serverMap newMap;

	newMap["listen"] = &serverParser::parseListen;
	newMap["cgi_ext"] = &serverParser::parseCgiExt;
	newMap["host"] = &serverParser::parseHost;
	newMap["server_name"] = &serverParser::parseName;
	newMap["root"] = &serverParser::parseRoot;
	newMap["allow_methods"] = &serverParser::parseMethods;
	newMap["index"] = &serverParser::parseIndex;
	newMap["autoindex"] = &serverParser::parseAutoindex;
	newMap["error_page"] = &serverParser::parseErrorPages;
	newMap["max_body_size"] = &serverParser::parseMaxBodySize;
	newMap["upload_dir"] = &serverParser::parseUploadDir;
	newMap["redirect_url"] = &serverParser::parseRedirectUrl;


	return newMap;
}

serverMap	serverParser::_serverFunctions = serverParser::_initServerFunctions();

serverParser::serverParser(void) : baseParser<VirtualServer>() {
	context.listen = 0;
	context.autoindex = false; // the default for nginx is also false
	context.server_name = "";
	context.root = "";
	context.max_body_size = 0;
};

serverParser::~serverParser(void) {};

/**
 * Main loop:
 * 
 * Check if token is in serverMap (the list of supported directives) or if it's 'location'
 * 		
 * 	no: scenario a & b
 * 	scenario a: token is not in serverMap, it is 'location', so it's a directive but one that is not handled like the other directives.
 * 		a1. if there's a directive in memory, add the directive to the server
 * 		a2. parse location and add to vector of locations
 * 	
 *	scenario b: token is not in serverMap and is also not 'location', so it's an argument to a directive
 * 		b1. push token to args
 * 
 * 	yes: scenario c
 * 	scenario c: token is in serverMap, so it's a directive
 * 		c1. if directive is not empty, add the directive in memory to the server configuration
 * 		c2. store value of token in directive
 */
VirtualServer	serverParser::parseServer(std::vector<std::string> &tokens, std::vector<std::string>::iterator &i) {

	std::vector<std::string>	args;
	std::string					directive = "";

	i++; // The token after "server" has to be an opening bracket
	if (i == tokens.end() || *i != "{")
		throw std::runtime_error("Invalid server configuration: missing opening bracket");
	
	i++; // We don't want to parse the opening bracket so increment once more
	
	while (i != tokens.end() && *i != "}") {
		if (_serverFunctions.find(*i) == _serverFunctions.end()) { // checks if token is not in serverMap
			//scenario a: token == "location" -> add result from locationParser to location vector
			if (*i == "location") {
				if (directive != "")
					addDirective(directive, args, *i);
				parseLocation(tokens, i);
				continue ;
			}
			else // scenario b
				args.push_back(*i);
		}
		else { // scenario c
			if (directive != "")
				addDirective(directive, args, *i);
			else
				directive = *i;
		}
		i++;
	}
	if (directive != "") // if we still have a directve in memory after exiting the loop, this needs to be added before we can continue.
		addDirective(directive, args, "hoho");
	if (i == tokens.end())
		throw std::runtime_error("Invalid server configuration: missing closing bracket");
	checkHomeLocation(); // to make response handling easier and be able to send back a location, even if there isn't a match (by returning the "home" location. must have a home location for each server in order to do that)
	return context;
}

/** listen nginx documentation: https://nginx.org/en/docs/http/ngx_http_core_module.html#listen
 * listen formats
 * 		listen	address[:port]
 * 		listen	port
 * 
 * scenario 1: ':' is present
 * 		both address and port are specified. split token on ':' character. word 1 is the address, word 2 is the port
 * scenario 2: no ':'
 * 		only the address or only the port is specified. We need to find out which one it is.
 * 			scenario 2a: token is only digits -> token represents port
 * 			scenario 2b: token represents address. 80 will be used for the port. address token is only valid if:
 * 								token is only alphabetical characters --> is hostname
 * 								token made up of digits and periods --> is IP address
 * 								token is an asterisk <-- screw implementing this
 */

void	serverParser::parseListen(std::vector<std::string> &args) {
	std::string address;
	std::string	port;
	std::string::size_type pos;

	if (args.size() != 1 || context.listen != 0)
		throw std::runtime_error("Configuration error: invalid listen directive");
	
	pos = args[0].find_first_of(':');
	if (pos != std::string::npos) {
		// scenario 1
		context.ip = args[0].substr(0, pos);
		 //std::cout << B_MAGENTA << "address: " << context.ip << RST << std::endl;
		port = args[0].substr(pos + 1, args[0].size() - 1);
		 //std::cout << B_MAGENTA << "port: " << port << RST << std::endl;
		pos = context.ip.find_first_not_of("0123456789.");
		if (pos != std::string::npos)
			throw std::runtime_error("Configuration error: invalid listen directive - ip not a valid");
		
		for (size_t i = 0; i < port.length(); i++) {
			if (!isdigit(port[i]))
				throw std::runtime_error("Configuration error: invalid listen directive - port can only contain digits");
		}
		try {
			context.listen = std::stoul(port);
		} catch (std::exception &e) {
			throw std::runtime_error("Configuration error: invalid listen directive - port not a valid integer");
		}
		return ;
	}
	
	for (size_t i = 0; i < args[0].length(); i++) {
		if (!isdigit(args[0][i]))
			throw std::runtime_error("Configuration error: invalid listen directive - port can only contain digits");
	}
	try {
		context.listen = std::stoul(args[0]);
	} catch (std::exception &e) {
		throw std::runtime_error("Configuration error: invalid listen directive - port not a valid integer");
	}

}

// to do: plz ignore this empty function for now
void	serverParser::parseHost(std::vector<std::string> &args) {

}

// to do: multiple server names are allowed. ngin documentation server_name: https://nginx.org/en/docs/http/ngx_http_core_module.html#server_name
void	serverParser::parseName(std::vector<std::string> &args) {
	if (args.size() > 1)
		throw std::runtime_error("Invalid server name");
	context.server_name = args.front();
}

void	serverParser::addDirective(std::string &directive, std::vector<std::string> &args, std::string newDirective) {
	if (directive == ""|| !args.size())
		throw std::runtime_error("add directive error");

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

	// to do: think about what to do when there are multiple locations with the same path
}

/**
 * loop through locations, see if location with path "/" exists
 * if yes, return
 * if no, create location with path "/". Copy directives from context (representing a VirtulServer) to newly created "/" location and push location to back of _locations vector
 */
void	serverParser::checkHomeLocation(void) {

	for (std::vector<Location>::iterator it = context.locations.begin(); it != context.locations.end(); it++) {
		if ((*it).path == "/") // to do: set empty directives to the default settings
			return ;
	}

	Location	home;

	home.root = context.root;
	home.cgi_ext.assign(context.cgi_ext.begin(), context.cgi_ext.end());
	home.upload_dir = context.upload_dir;
	home.redirect_url = context.redirect_url;
	home.max_body_size = context.max_body_size;
	home.autoindex = context.autoindex;
 home.index.assign(context.index.begin(), context.index.end());
    home.methods.assign(context.methods.begin(), context.methods.end());
	home.error_pages = context.error_pages;
	// to do: set empty directives to the default settings

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
	    out << "\ncgi_ext: ";
    for (size_t i = 0; i < obj.cgi_ext.size(); i++) {
        out << obj.cgi_ext[i];
        if (i < obj.cgi_ext.size() - 1)
            out << " ";
    }
    out << std::endl;
	out << "upload_dir: " << obj.upload_dir << std::endl;
	out << "max_body_size: " << obj.max_body_size << std::endl;
	out << "autoindex: " << (!obj.autoindex ? "off" : "on") << std::endl;
	out << "index: ";
	for (size_t i = 0; i < obj.index.size(); i++)
		out << obj.index[i] << " ";
	out << "\nerror_pages:" << std::endl;
	for (std::map<unsigned int, std::string>::const_iterator it = obj.error_pages.begin(); it != obj.error_pages.end(); ++it)
			out << "	" << it->first << " " << it->second << std::endl;
	for(size_t i = 0; i < obj.locations.size(); i++)
		std::cout << obj.locations[i] << std::endl;
	return (out);
}
