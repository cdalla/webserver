#ifndef CONFIG_HPP
#define CONFIG_HPP

#include<iostream>
#include<vector>
#include<map>
#include<fstream>
#include<exception>
#include<sstream>

struct Location {
	std::string					path;
	std::vector<std::string>	methods;
	std::string					root;
	int							autoindex;
	std::string					cgi_path;
	std::string					upload_dir;
	std::vector<std::string>	index;
	int							redirect_cd;
	std::string					redirect_url;
	std::string					max_body_size;
};

struct VirtualServer {
	std::string					host;
	std::string					port;
	std::string					name;
	std::string					root;
	std::vector<std::string>	cgi_exit;
	std::map<int, std::string>	error_pages;
	std::vector<Location>		locations;

};

class Config {

private:
	const std::string _path;

public:
	std::vector<VirtualServer> servers;

	Config(const std::string &path);
	~Config();

	void parse();
	void parseServer(std::vector<std::string> &tokens, size_t &i);
	void paresHostPort(std::string &host, std::string &port, std::string &token);
	void parseCgiExt(std::vector<std::string> &cgi_ext, const std::vector<std::string> &tokens, size_t &i);
	void parseErrorPage(std::map<int, std::string> &error_pages, std::vector<std::string> &tokens, size_t &i);
	void parseLocation(std::vector<Location> &locations, const std::vector<std::string> &tokens, size_t &i);

	bool check_word(const std::string &word);

	void CheckPort(std::string port);
	void CheckServerName(std::string serverName);
	void CheckPath(std::string path);
	void CheckAllow(std::vector<std::string> methods);
	void CheckLocation(std::vector<Location> location);
	void CheckClientMaxBodySize (std::string maxBodySize);
	void display();
};

#endif