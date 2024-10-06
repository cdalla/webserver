#include "webserver.hpp"

int	main(int argc, char **argv) {

	if (argc != 2) {
		std::cout << "Exactly 1 argument expected" << std::endl;
	}

	Webserver webserv(argv[1]);
	try {
		webserv.run();
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

	return (0);
}

/**
 * instantiate webserver instance
 * 
 */