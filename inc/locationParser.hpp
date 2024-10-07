#ifndef LOCATIONPARSER_HPP
# define LOCATIONPARSER_HPP

#include "ConfigStructs.hpp"
#include "baseParser.hpp"

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