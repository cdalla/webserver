#ifndef BASEPARSER_HPP
# define BASEPARSER_HPP

#include "structs.hpp"

/**
 * 		!!! this is only for parsing configuration files
 * 		ABOUT BASEPARSER CLASS
 * 
 * There is a lot of overlap between parsing a server and parsing a location. To avoid redundant functions,
 * we have a template base class for parsing a server or a location. This baseParser contains the functions for
 * parsing directives that can appear in the context of a server and in the context of a location.
 * 
 * The baseParser class also contains a template object 'context'. If the baseParser class is instantiated
 * as the base of a serverParser, this object will have type VirtualServer. If it's instantiated as the base
 * of a locationParser, this object will have type Location.
 * 
 */
  template <typename T>
class baseParser {
	public:

		virtual ~baseParser(void){};

		class ConfigException;
		T context;
	
	protected:
		void	parseCgiPass(std::vector<std::string> &args);
		void	parseRoot(std::vector<std::string> &args);
		void	parseMethods(std::vector<std::string> &args);
		void	parseAutoindex(std::vector<std::string> &args);
		void	parseErrorPages(std::vector<std::string> &args);
		void	parseIndex(std::vector<std::string> &args);
		void	parseUploadDir(std::vector<std::string> &args);
		void	parseRedirectUrl(std::vector<std::string> &args);
		void	parseMaxBodySize(std::vector<std::string> &args);

};

#endif