#include "baseParser.hpp"
#include "colours.hpp"
#include <iostream>

  template<typename T>
class baseParser<T>::ConfigException : public std::exception {
	public:
		virtual const char* what() const throw() { return ("Config exception: invalid configuration"); };
};

  template <typename T>
void	baseParser<T>::parseCgiExt(std::vector<std::string> &args) {
	if (args.empty()) {
        throw std::runtime_error("Configuration error: CGI extensions cannot be empty");
    }
    
    context.cgi_ext.clear();
    
    for (std::vector<std::string>::iterator it = args.begin(); it != args.end(); ++it) {
        if (*it != ".py" && *it != ".sh") {
            throw std::runtime_error("Configuration error: only bash or python CGI is supported (.sh, .py)");
        }
        context.cgi_ext.push_back(*it);
    }
}

  template <typename T>
void	baseParser<T>::parseUploadDir(std::vector<std::string> &args) {
	if (args.size() != 1) {
		std::cout << "parseUploadDir\n";
		throw ConfigException();
	}
	context.upload_dir = args[0];
}


  template <typename T>
void	baseParser<T>::parseRoot(std::vector<std::string> &args) {
	if (args.size() != 1) {
		std::cout << "parseRoot \n";
		throw ConfigException();
	}
	context.root = args[0];
}

  template <typename T>
void	baseParser<T>::parseMethods(std::vector<std::string> &args) {
	for (std::vector<std::string>::iterator i = args.begin(); i != args.end(); i++) {
		if (*i == "GET" || *i == "POST" || *i == "DELETE")
			context.methods.push_back(*i);
		else {
			throw ConfigException();
		}
	}
	if (!context.methods.size())
		throw ConfigException();
}

  template <typename T>
void	baseParser<T>::parseAutoindex(std::vector<std::string> &args) {
	if (args.size() != 1) {
		throw ConfigException();
	}
	if (args[0] == "on")
		context.autoindex = true;
	else if (args[0] != "off")
		throw ConfigException();
}

  template <typename T>
void	baseParser<T>::parseIndex(std::vector<std::string> &args) {

	for (size_t i = 0; i < args.size(); i++) {
		context.index.push_back(args[i]);
	}
}

  template <typename T>
void	baseParser<T>::parseErrorPages(std::vector<std::string> &args) {
	unsigned int	code;
	std::string		path;

	if (args.size() != 2) {
		std::cout << "parseErrorPages";
		throw ConfigException();
	}
	for (std::string::iterator len = (args[0]).begin(); len != (args[0]).end(); len++) {
		if (!isdigit(*len))
			throw ConfigException();
	}
	try {
		code = std::stoul(args[0]);
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		throw ConfigException();
	}
	path = args[1];
	context.error_pages[code] = path;
}

//  template <typename T>
// void	baseParser<T>::parseUploadDir(std::vector<std::string> &args) {
// 	if (args.size() != 1) {
// 		std::cout << "parseUploadDir ConfigException\n";
// 		throw ConfigException();
// 	}
// 	context.upload_dir = args[0];
// }

  template <typename T>
void	baseParser<T>::parseRedirectUrl(std::vector<std::string> &args) {

	if (args.size() != 1) {
		std::cout << "parseRedirectUrl ConfigException\n";
		throw ConfigException();
	}
	context.redirect_url = args[0];

}

  template <typename T>
void	baseParser<T>::parseMaxBodySize(std::vector<std::string> &args) {

	if (args.size() != 1) {
		std::cout << "parseMaxBodySize ConfigException\n";
		throw ConfigException();
	}
	//context.max_body_size = args[0];
	std::string str = args[0];
	
	/*
	- no letter bytes
	-K kilo
	-M meg
	-G giga
	-T tera
	-Z ziliobyte
	*/
	size_t pos = str.find_first_not_of("0123456789");
	if (pos == std::string::npos){
		context.max_body_size = stoi(str);
		return;
	}
	if (pos != str.size() - 1 || (str[pos] != 'K' && str[pos] != 'M' && str[pos] != 'G' ))
	{
		//std::cout << "SUCA" << std::endl;
		return; //error
	}
	std::string res = str.substr(0, pos);
	switch (str[pos])
	{
	case 'K':
		res.append("000");
		break;
	case 'M':
		res.append("000000");
		break;
	case 'G':
		res.append("000000000");
		break;
	}
	context.max_body_size = stoi(res);


}