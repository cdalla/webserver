#ifndef BASEPARSER_HPP
# define BASEPARSER_HPP

#include "ConfigStructs.hpp"

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