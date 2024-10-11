#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <string>
#include <vector>
#include "ConfigStructs.hpp"
#include "serverParser.hpp"


/**
 * 		ABOUT CONFIG, SERVERPARSER, LOCATIONPARSER - WHAT IS BEING CALLED BY WHAT
 * 
 *  --- now we are in main.cpp, this is where a Webserver is instantiated
 * class Webserver has an instance of class Config as member attribute. class Config has a member function called parseConfig.
 * 
 * --- now we are in webserver.cpp
 * The constructor for class Webserver calls config.parseConfig()
 * 
 * --- now we are in config.cpp --> from here on it's a loop
 * if parseConfig encounters a token 'server', it locally instantiates an instance of serverParser and calls it 'parser'. serverParser has a member function
 * called parseServer, which returns a filled VirtualServer struct.
 * parseConfig calls parser.parseServer()
 * 
 * --- now we are in serverParser.cpp
 * if parseServer encounters a token 'location', it first calls its own serverParser::parseLocation(), to check
 * if the location already exists.
 * 
 * In serverParser::parseLocation(), a local instance of a locationParser called 'parser' is instantiated. If the location we are
 * parsing does not yet exist, it calls parser.parseLocation(), which is a locationParser::parseLocation()
 * 
 * --- now we are in locationParser.cpp
 * That's it, this is as far as the nesting goes. locationParser does not call any parsing classes.
 * 
 * After locationParser::parseLocation(), we are back in serverParser::parseServer(). If parseServer() encounters another unique
 * 'location' token, it will call locationParser::parseLocation() again. If not, it will return to config::parseConfig(), closing the loop for this one server configuration
 */

class Config {
	public:
		Config(const char *path); // also create a void constructor that doesn't read path from arguments, but that uses default path 
		~Config(void);

		class ConfigException: public std::exception {
			public:
				virtual const char* what() const throw() { return ("Server config exception: invalid configuration"); };
		};

		std::vector<VirtualServer>  servers; // should proba ly rename to "configs" or something
		std::vector<unsigned int> listens;

		void			parseConfig(void);
	
	private:
		const char*	_path;
};

std::ostream&   operator<<(std::ostream& out, Config const &obj);

#endif