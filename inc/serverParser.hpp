#ifndef SERVERPARSER_HPP
# define SERVERPARSER_HPP

#include "ConfigStructs.hpp"
#include "baseParser.hpp"

class serverParser : public baseParser<VirtualServer> {
	public:
		serverParser(void);
		~serverParser(void);

		VirtualServer	parseServer(std::vector<std::string> &tokens, std::vector<std::string>::iterator &i);

	private:
		// parser functions
		/*
		from NGINX (https://docs.nginx.com/nginx/admin-guide/web-server/web-server/) if the listen directive is not included at all, the “standard” port is 80/tcp and the “default” port is 8000/tcp, depending on superuser privileges.*/
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