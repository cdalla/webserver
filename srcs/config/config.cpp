#include <iostream>
#include <fstream>

#include "config.hpp"
#include "colours.hpp"

Config::Config(const char *path): _path(path) {};

Config::~Config(void) {};

/**
 * Main loop
 * 	if token == "server"
 * 		instantiate serverParser instance called parser
 * 		pass tokens and current location to the parseServer function of parser, and push the result to servers
 * 	increment i, and we're ready to parse and add the next server
 */
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
	if (!servers.size())
		throw std::runtime_error("Configuration error: no servers in configuration file");

	for (int i =0; i < servers.size(); i++)
	{
		if (servers[i].listen == 0)
			throw std::runtime_error("Configuration error: one or more servers is missing listen directive");
		// std::cout << servers[i] << std::endl;
	}
};

std::ostream&   operator<<(std::ostream& out, Config const &obj) {

	out << CYAN "Config\n" RST;
	out << U_WHT << "Servers:\n" << RST;
	for (size_t i = 0; i < obj.servers.size(); i++)
		out << obj.servers[i];
	return (out);
}

