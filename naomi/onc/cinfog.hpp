#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <string>
#include <vector>
#include <map>

struct Location {
	std::string					path;
	std::string					root;
	std::string					cgi_pass;
	std::string					upload_dir;
	std::string					redirect_url;
	std::string					max_body_size;
	std::vector<std::string>	index; //for if requested resource is directory
	std::vector<std::string>	methods;
};

struct VirtualServer
{
	int							listen; //port 
	unsigned int				host; //ip address
	std::string					server_name;
	std::string					root;
	std::vector<std::string>	methods;
	std::map<unsigned int, std::string>	error_pages;
	std::vector<Location>		locations;
};

class Config {
	public:
		Config(const char *path); // also create a void constructor that doesn't read path from arguments, but that uses default path 
		~Config(void);

		std::vector<VirtualServer>  servers;
		std::map<unsigned int, int>	listens;

		void readConfig(void);
		void parseConfig(void);

		VirtualServer	parseServer(std::vector<std::string> &tokens, std::vector<std::string>::iterator &i);
		Location		parseLocation(std::vector<std::string> &tokens, std::vector<std::string>::iterator &i);

	private:
		const char*	_path;
		std::vector<std::string>	_keys;
		// std::map<std::string, std::string> _keys;

};

#endif