#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <string>
#include "ConfigStructs.hpp"
#include "serverParser.hpp"


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
		// std::map<unsigned int, int>	listens;

		void			parseConfig(void);
	
	private:
		const char*	_path;
};

std::ostream&   operator<<(std::ostream& out, Config const &obj);

#endif