#include "webserver.hpp"

int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cout << "Exactly one argument expected" << std::endl;
		return (0);
	}

	Webserver webserv(argv[1]);
	try {
		webserv.run();
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	
	return(0);
}