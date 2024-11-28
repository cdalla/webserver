#include <iostream>

#include "locationParser.hpp"
#include "baseParser.cpp"
#include "colours.hpp"

/*

- default settings?
- errors/fatal errors/misconfiguration
*/

locationParser::locationParser(void) : baseParser<Location>() {
	context.path = "";
	context.root = "";
	context.upload_dir = "";
	context.redirect_url = "";
	context.max_body_size = 0;
}

locationParser::~locationParser(void) {}

locationMap	locationParser::_initLocationFunctions() {

	locationMap newMap;

	newMap["cgi_ext"] = &locationParser::parseCgiExt;
	newMap["upload_dir"] = &locationParser::parseUploadDir;
	newMap["root"] = &locationParser::parseRoot;
	newMap["allow_methods"] = &locationParser::parseMethods;
	newMap["index"] = &locationParser::parseIndex;
	newMap["autoindex"] = &locationParser::parseAutoindex;
	newMap["error_page"] = &locationParser::parseErrorPages;
	newMap["max_body_size"] = &locationParser::parseMaxBodySize;


	return newMap;
}

locationMap	locationParser::_locationFunctions = locationParser::_initLocationFunctions();

Location	locationParser::parseLocation(std::vector<std::string> &tokens, std::vector<std::string>::iterator &i) {

	locationMap::iterator	it;
	std::string				directive = "";
	std::vector<std::string>	args;

	i++;
	if (i == tokens.end() || *i == "{")
		throw std::runtime_error("Invalid location configuration: missing location path");
	if ((*i).at(0) != '/')
	{
		//std::cout << (*i) << std::endl;
		throw std::runtime_error("Invalid location configuration: missing opening slash path");
	}
	context.path = *i;
	
	i++;
	if (i == tokens.end() || *i != "{")
		throw std::runtime_error("Invalid location configuration: missing opening bracket");
	i++;
	while (i != tokens.end() && *i != "}") {
		if (_locationFunctions.find(*i) == _locationFunctions.end()) {
			args.push_back(*i);
		}
		else {
			if (directive != "")
				addDirective(directive, args, *i);
			else
				directive = *i;
		}
		i++;
	}
	if (directive != "") // to do: wonder if it's even possible to make it out of the loop above without a directive (if there's a way you could get that with a valid config file)
		addDirective(directive, args, "");
	if (*i != "}")
		throw std::runtime_error("Invalid location configuration: missing closing bracket");
	i++;
	return context;
}

void	locationParser::addDirective(std::string &directive, std::vector<std::string> &args, std::string newDirective) {
		if (directive == ""|| !args.size())
			throw std::runtime_error("Invalid configuration: directive " + directive);
		(this->*locationParser::_locationFunctions[directive])(args);
		args.clear();
		directive = newDirective;
}

std::ostream&   operator<<(std::ostream& out, Location const &obj) {

	out << U_WHT << "Location" << RST << std::endl;
	out << "	path: " << obj.path << std::endl;
	out << "	root: " << obj.root << std::endl;
	out << "	methods: ";
    for (std::list<std::string>::const_iterator it = obj.methods.begin(); it != obj.methods.end(); ++it) {
        out << *it << " ";
	}
	out << "\n";
	out << "    cgi_ext: ";
    for (std::list<std::string>::const_iterator it = obj.cgi_ext.begin(); it != obj.cgi_ext.end(); ++it) {
        out << *it << " ";
    }
    out << "\n";
	if (obj.upload_dir != "")	
		out << "	upload_dir: " << obj.upload_dir << std::endl;
	if (obj.redirect_url != "")
		out << "	redirect_url: " << obj.redirect_url << std::endl;
	if (obj.max_body_size)
		out << "	max_body_size: " << obj.max_body_size << std::endl;
	out << "	autoindex: " << (!obj.autoindex ? "off" : "on") << std::endl;
	if (obj.index.size())
	{
		out << "	index: ";
		for (std::list<std::string>::const_iterator it = obj.index.begin(); it != obj.index.end(); ++it) {
        out << *it << " ";
        }
		out << "\n";
	}
	out << "	error_pages:" << std::endl;
	for (std::map<unsigned int, std::string>::const_iterator it = obj.error_pages.begin(); it != obj.error_pages.end(); ++it) {
			out << "		" << it->first << " " << it->second << std::endl;
	}
	return (out);
}