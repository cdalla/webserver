#include "file.hpp"
#include <sys/stat.h>

File::File(int fd, Webserver* ptr, Client * client): _main(ptr), _client(client)
{
   this->set_fd(fd);
    
    try {
        // Read the entire file content immediately instead of using epoll
        struct stat st;
        if (fstat(fd, &st) == -1) {
            throw WebservException("Failed to get file stats: " + std::string(strerror(errno)));
        }
        
        std::cout << "File size: " << st.st_size << " bytes" << std::endl;

        // Allocate a buffer for the entire file
        std::string content;
        content.reserve(st.st_size);
        char buffer[4096];
        ssize_t bytes_read;
        size_t total_bytes = 0;
        
        while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
            content.append(buffer, bytes_read);
            total_bytes += bytes_read;
        }
        
        if (bytes_read == -1) {
            throw WebservException("Failed to read file: " + std::string(strerror(errno)));
        }
        
        // Debug content
        std::cout << "Read " << total_bytes << " bytes from file" << std::endl;
        std::cout << "Content: [" << content << "]" << std::endl;
        
        // Store the content in the client
        if (_client) {
            std::cout << "Setting file content in client" << std::endl;
            _client->file_content = content;
        }
        
        // Close the file descriptor since we've read everything
        close(fd);
    }
    catch (const std::exception& e) {
        close(fd);
        throw;
    }
}

bool File::consume(int event_type)
{
 if (!_client) {
        _main->removeFd(_fd, FILES);
        return true;
    }

    if (event_type != IN) {
        return false;
    }

    reset_last_activity();
    char buff[MAX_BUFF];
    std::string content;
    ssize_t total_bytes = 0;
    
    while (true) {
        ssize_t bytes_read = read(_fd, buff, MAX_BUFF - 1);
        
        if (bytes_read > 0) {
            buff[bytes_read] = '\0';
            content.append(buff, bytes_read);
            total_bytes += bytes_read;
        }
        else if (bytes_read == 0) {
            // End of file reached
            _client->file_content = content;
            _main->removeFd(_fd, FILES);
            return true;
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No more data available right now
            if (total_bytes > 0) {
                _client->file_content = content;
            }
            return false;
        }
        else {
            // Error occurred
            _main->removeFd(_fd, FILES);
            return true;
        }
    }
}


/*
void responseHandler::_handlePage(std::string path)
{

    if (_client->request.method == "POST" || _client->request.method == "DELETE")
	{
		_handleError(405);
		return;
	}

    //////////////////////////////////////////
    PART CHANGED BY CARLO
   
    
    //////////////////////////////////////////

	_determineType(path);//idk if u need this
	_response = "200 OK\r\n";
	_response.append("Content-Type: ");
	_response.append(_content_type);
	_response.append("\r\n");
	_response.append("Content-Length: ");
	_response.append(std::to_string(_body.length()));
	_response.append("\r\n\r\n");
	_response.append(_body);
}
*/