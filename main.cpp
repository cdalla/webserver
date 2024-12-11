#include "webserver.hpp"
#include <signal.h>

void sig_handler(int signal)
{
	(void) signal;
	print_msg("Stopped by signal!");
	exit(0);
}

int main(int argc, char **argv)
{
	if (argc > 2) 
	{
		print_error("Too many arguments provided");
		return (0);
	}

	signal(SIGINT, sig_handler);

	std::string config_file = "configurations/default.conf";
	if (argc ==  2)
		config_file = std::string(argv[1]);
	Webserver webserv(config_file);
	try 
	{
		webserv.run();
	} 
	catch (std::exception &e) 
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}
	
	return(0);
}