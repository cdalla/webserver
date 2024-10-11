#ifndef LOCATIONPARSER_HPP
# define LOCATIONPARSER_HPP

#include "ConfigStructs.hpp"
#include "baseParser.hpp"

/**
 * 		ABOUT LOCATIONPARSER CLASS
 * locationParser inherits the shared parsing functions and an object called context from baseParser.
 * In a locationParser, this object (context) has type Location.
 * 
 * The main entry point for locationParser is the function parseLocation. It takes a vector of tokens
 * and the our current location parsing these tokens (i). Par
 */

class locationParser: public baseParser<Location> {
	public:
		locationParser(void);
		~locationParser(void);

		Location		parseLocation(std::vector<std::string> &tokens, std::vector<std::string>::iterator &i);
	
	private:
		void	addDirective(std::string &directive, std::vector<std::string> &args, std::string newDirective);
		static locationMap	_locationFunctions;
		static locationMap	_initLocationFunctions();
};

std::ostream&   operator<<(std::ostream& out, Location const &obj);

#endif