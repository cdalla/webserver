#include "file.hpp"


File::File(std::string filename, Webserver* ptr, Client* client): _main(ptr), _client(client), _buff(""), _file_size(0)
{
	std::cout << "FILEHANLDER!!!!!!!!!!!!!!!11"<<std::endl;
	reset_last_activity();
	_fd = open(filename.c_str(), O_RDONLY);
	if (_fd < 0)
		throw WebservException("Failed to open: " + std::string(strerror(errno)));
	if (pipe(_pipe) == -1)
		throw WebservException("Failed to pipe: " + std::string(strerror(errno)));
    make_socket_non_blocking(_fd);
	int flags;
	flags |= O_NONBLOCK;
    if (fcntl(_pipe[0], F_SETFL, flags) == -1) 
		throw WebservException("Failed to fcntl set_flag: " + std::string(strerror(errno)));
	if (fcntl(_pipe[1], F_SETFL, flags) == -1) 
		throw WebservException("Failed to fcntl set_flag: " + std::string(strerror(errno)));
    _main->addFdToPoll(_pipe[0], _main->get_EpollFd(FILES), EPOLLIN);
    _main->addFdToMap(_pipe[0], this);
	_main->addFdToPoll(_pipe[1], _main->get_EpollFd(FILES), EPOLLOUT);
    _main->addFdToMap(_pipe[1], this);
	std::cout<< "pipe in = " << _pipe[0] << " pipe out = " << _pipe[1] << std::endl;
}
bool File::consume(int event_type)
{
	if (IN)
	{
		std::cout << "IN PIPE" << std::endl;
		char buff[MAX_BUFF];
		ssize_t bytes = read(_pipe[0], buff, MAX_BUFF);
		if (bytes < 0)
			throw WebservException("Failed to read pipe in: " + std::string(strerror(errno)));
		else if (bytes == 0)
		{
			close(_pipe[0]);
			_client->status = "";
			return false;
		} 
		_client->file_content.append(buff);
		std::cout << "IN: " << buff << std::endl;
		
	}
	else if (OUT)
	{
		char buff[MAX_BUFF];
		std::cout << "OUT PIPE" << std::endl;
		if (_fd != -1)
		{
			ssize_t bytes_r = read(_fd, buff, MAX_BUFF);
			if (bytes_r < 0)
				throw WebservException("Failed to read file in: " + std::string(strerror(errno)));
			else if (bytes_r == 0)
			{
				std::cout << "read zero bytes" << std::endl;
				close(_fd);
				return true;
				//_main->removeFd(_pipe[1], FILES);
				//_pipe[1] = -1;
				_fd = -1;
			}
		}
		if (_fd != -1)
		{
			ssize_t bytes_w = write(_pipe[1], buff, sizeof(buff));
			if (bytes_w < 0)
				throw WebservException("Failed to write pipe out: " + std::string(strerror(errno)));
			//std::cout << "pipe: " << _pipe[1] << " OUT: "  << buff << std::endl;
		}
	}
	else
		std::cout << "ERROR EVENT" << std::endl;
   return false;
}

void File::input()
{
	reset_last_activity();
	//std::cout << "IN PIPE" << std::endl;
	ssize_t bytes_r = read(_pipe[0], _buff, MAX_BUFF);
	// std::cout << "bytes_r = " << bytes_r << std::endl;
	if (bytes_r < 0)
		throw WebservException("Failed to read pipe in: " + std::string(strerror(errno)));
	else if (bytes_r == 0)
	{
		_client->status.clear();
		_main->removeFd(_pipe[0], FILES, 1);
	}
	else
	{
		_client->file_content.append(_buff, bytes_r);
		if (_client->file_content.size() == (size_t)_file_size)
		{
			std::cout << "file size: " << _file_size << std::endl;
			_client->status.clear();
			_main->removeFd(_pipe[0], FILES, 1);
		}
    	//std::cout << "file content in file_han: \n" << _client->file_content << std::endl;
//		std::cout << "IN: " << _buff << std::endl;
	}
}

void File::output()
{
	reset_last_activity();
	//std::cout << "OUT PIPE" << std::endl;
	ssize_t bytes_r = read(_fd, _buff, MAX_BUFF);
	if (bytes_r < 0)
		throw WebservException("Failed to read file in: " + std::string(strerror(errno)));
	else if (bytes_r == 0)
	{
		std::cout << "read zero bytes" << std::endl;
		_main->removeFd(_pipe[1], FILES, 0); //remove the fd but not the handler
	}
	else
	{
		ssize_t bytes_w = write(_pipe[1], _buff, bytes_r);
		//std::cout << "bytes_W = " << bytes_w << std::endl;
		if (bytes_w < 0)
			throw WebservException("Failed to write pipe out: " + std::string(strerror(errno)));
		else
			_file_size += bytes_w;
		//std::cout << "pipe: " << _pipe[1] << " OUT: "  << _buff << std::endl;
	}
}

// #include "file.hpp"
// #include <sys/stat.h>

// File::File(int fd, Webserver* ptr, Client * client): _main(ptr), _client(client)
// {
//    this->set_fd(fd);
    
//     try {
//         // Read the entire file content immediately instead of using epoll
//         struct stat st;
//         if (fstat(fd, &st) == -1) {
//             throw WebservException("Failed to get file stats: " + std::string(strerror(errno)));
//         }
        
//         std::cout << "File size: " << st.st_size << " bytes" << std::endl;

//         // Allocate a buffer for the entire file
//         std::string content;
//         content.reserve(st.st_size);
//         char buffer[4096];
//         ssize_t bytes_read;
//         size_t total_bytes = 0;
        
//         while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
//             content.append(buffer, bytes_read);
//             total_bytes += bytes_read;
//         }
        
//         if (bytes_read == -1) {
//             throw WebservException("Failed to read file: " + std::string(strerror(errno)));
//         }
        
//         // Debug content
//         std::cout << "Read " << total_bytes << " bytes from file" << std::endl;
//         std::cout << "Content: [" << content << "]" << std::endl;
        
//         // Store the content in the client
//         if (_client) {
//             std::cout << "Setting file content in client" << std::endl;
//             _client->file_content = content;
//         }
        
//         // Close the file descriptor since we've read everything
//         close(fd);
//     }
//     catch (const std::exception& e) {
//         close(fd);
//         throw;
//     }
// }

// bool File::consume(int event_type)
// {
//  if (!_client) {
//         _main->removeFd(_fd, FILES);
//         return true;
//     }

//     if (event_type != IN) {
//         return false;
//     }

//     reset_last_activity();
//     char buff[MAX_BUFF];
//     std::string content;
//     ssize_t total_bytes = 0;
    
//     while (true) {
//         ssize_t bytes_read = read(_fd, buff, MAX_BUFF - 1);
        
//         if (bytes_read > 0) {
//             buff[bytes_read] = '\0';
//             content.append(buff, bytes_read);
//             total_bytes += bytes_read;
//         }
//         else if (bytes_read == 0) {
//             // End of file reached
//             _client->file_content = content;
//             _main->removeFd(_fd, FILES);
//             return true;
//         }
//         else if (errno == EAGAIN || errno == EWOULDBLOCK) {
//             // No more data available right now
//             if (total_bytes > 0) {
//                 _client->file_content = content;
//             }
//             return false;
//         }
//         else {
//             // Error occurred
//             _main->removeFd(_fd, FILES);
//             return true;
//         }
//     }
// }


// /*
// void responseHandler::_handlePage(std::string path)
// {

//     if (_client->request.method == "POST" || _client->request.method == "DELETE")
// 	{
// 		_handleError(405);
// 		return;
// 	}

//     //////////////////////////////////////////
//     PART CHANGED BY CARLO
   
    
//     //////////////////////////////////////////

// 	_determineType(path);//idk if u need this
// 	_response = "200 OK\r\n";
// 	_response.append("Content-Type: ");
// 	_response.append(_content_type);
// 	_response.append("\r\n");
// 	_response.append("Content-Length: ");
// 	_response.append(std::to_string(_body.length()));
// 	_response.append("\r\n\r\n");
// 	_response.append(_body);
// }
// */