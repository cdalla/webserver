#ifndef SERVERPARSER_HPP
# define SERVERPARSER_HPP

#include "baseParser.hpp"

/**
 * 		ABOUT SERVERPARSER CLASS
 * serverParser inherits the shared parsing functions and an object called context from baseParser.
 * In a serverParser, this object (context) has type VirtualServer.
 * 
 * The main entry point for serverParser is the function parseServer. This function takes our vector of tokens and
 * our current location parsing these tokens (i). parseServer fills the object called context (in this case context will be a VirtualServer)
 * and returns it.
 * 
 * Some directives can appear in the context of a server, but not in that of a location. The functions
 * for parsing those directives are declared in the serverParser class.
 * 
 * _serverFunctions is a map of function pointers, pointing to all of the functions needed to parse a server.
 */

class serverParser : public baseParser<VirtualServer> {
	public:
		serverParser(void);
		~serverParser(void);

		VirtualServer	parseServer(std::vector<std::string> &tokens, std::vector<std::string>::iterator &i);

	private:
		// to do: from NGINX (https://docs.nginx.com/nginx/admin-guide/web-server/web-server/) if the listen directive is not included at all, the “standard” port is 80/tcp and the “default” port is 8000/tcp, depending on superuser privileges.
		
		/* server-specific parsing functions */
		void	parseListen(std::vector<std::string> &args);
		void	parseHost(std::vector<std::string> &args);
		void	parseName(std::vector<std::string> &args);
		void	parseLocation(std::vector<std::string> &tokens, std::vector<std::string>::iterator &i);
		void	checkHomeLocation(void);

		//member attributes
		void	addDirective(std::string &directive, std::vector<std::string> &args, std::string newDirective);
		static serverMap	_serverFunctions;
		static serverMap	_initServerFunctions();
};

std::ostream&   operator<<(std::ostream& out, VirtualServer const &obj);

#endif