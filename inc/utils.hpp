#ifndef UTILS_HPP
# define UTILS_HPP

void			make_socket_non_blocking(int socket_fd);
std::string		get_URI_prefix(std::string const &URI);
void            print_msg(std::string param);
void            print_error(std::string param);

#endif