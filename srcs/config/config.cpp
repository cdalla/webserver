#include <iostream>
#include <fstream>

#include "config.hpp"
#include "colours.hpp"
#include "serverParser.hpp"

int	main(int argc, char **argv) {

	Config config(argv[1]);
	return (0);
};

Config::Config(const char *path): _path(path) {
	parseConfig();
};

Config::~Config(void) {
	
};

void	Config::parseConfig(void) {

	std::ifstream ifs(_path);

	if (!ifs.is_open()) {
		std::cerr << "File does not exist: " << _path << std::endl;
		exit(1);
	};

	std::vector<std::string> tokens;
	std::string token;

	while (ifs >> token) {
		tokens.push_back(token);
	};

	ifs.close();

	std::vector<std::string>::iterator i = tokens.begin();

	while (i != tokens.end()) {
		if (*i == "server") {
			serverParser parser;
			try {
				servers.push_back(parser.parseServer(tokens, i));
			} catch (std::exception &e) {
				std::cerr << e.what() << std::endl;
				exit(0);
			}
		}
		i++;
	}
	for (size_t i = 0; i < servers.size(); i++)
		std::cout << servers[i] << std::endl;
};

std::ostream&   operator<<(std::ostream& out, Config const &obj) {

	// out << CYAN "HTTP Request\n" RST;
	// out << "  " U_WHT "General:\n" RST;
	// out << "    Method: " << obj.method << "\n";
	// out << "    URL: " << obj.getURL() << "\n";
	// out << "  " U_WHT "Parsing variables:\n" RST;
	// out << "    exists: " << (obj.exists? "true" : "false") << "\n";
	// out << "    resource: " << obj.resource << "\n";
	// out << "    extension: " << obj.extension << "\n";
	// out << "    _path: " << obj.getPath() << "\n";
	return (out);
}

