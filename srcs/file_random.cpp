#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <iostream>
class PipeReader {
private:
    int epoll_fd;
    int pipe_read_fd;
    int pipe_write_fd;
    bool is_running;
    static const int BUFFER_SIZE = 4096;
public:
    PipeReader() : epoll_fd(-1), pipe_read_fd(-1), pipe_write_fd(-1), is_running(false) {
        // Create epoll instance
        epoll_fd = epoll_create1(0);
        if (epoll_fd == -1) {
            throw std::runtime_error("Failed to create epoll instance");
        }
        // Create pipe
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            close(epoll_fd);
            throw std::runtime_error("Failed to create pipe");
        }
        pipe_read_fd = pipefd[0];
        pipe_write_fd = pipefd[1];
        // Set pipe read end to non-blocking
        int flags = fcntl(pipe_read_fd, F_GETFL, 0);
        fcntl(pipe_read_fd, F_SETFL, flags | O_NONBLOCK);
        // Add pipe read end to epoll
        epoll_event ev;
        ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET;  // Edge-triggered mode
        ev.data.fd = pipe_read_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_read_fd, &ev) == -1) {
            cleanup();
            throw std::runtime_error("Failed to add to epoll");
        }
    }
    void processFile(const std::string& filename) {
        int file_fd = open(filename.c_str(), O_RDONLY);
        if (file_fd == -1) {
            throw std::runtime_error("Failed to open file");
        }
        is_running = true;
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        // Read from file and write to pipe
        while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
            ssize_t bytes_written = write(pipe_write_fd, buffer, bytes_read);
            if (bytes_written == -1) {
                close(file_fd);
                throw std::runtime_error("Failed to write to pipe");
            }
        }
        close(file_fd);
        // Close write end of pipe - this is crucial!
        close(pipe_write_fd);
        pipe_write_fd = -1;
        // Event loop
        epoll_event events[10];
        while (is_running) {
            int nfds = epoll_wait(epoll_fd, events, 10, -1);
            if (nfds == -1) {
                if (errno == EINTR) continue;
                throw std::runtime_error("epoll_wait failed");
            }
            for (int n = 0; n < nfds; ++n) {
                if (events[n].data.fd == pipe_read_fd) {
                    if (events[n].events & EPOLLIN) {
                        // Read from pipe
                        while (true) {
                            bytes_read = read(pipe_read_fd, buffer, BUFFER_SIZE);
                            if (bytes_read == -1) {
                                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                    // No more data available right now
                                    break;
                                }
                                throw std::runtime_error("Failed to read from pipe");
                            }
                            if (bytes_read == 0) {
                                // EOF reached
                                is_running = false;
                                break;
                            }
                            // Process the data here
                            std::cout.write(buffer, bytes_read);
                        }
                    }
                    if (events[n].events & (EPOLLHUP | EPOLLRDHUP)) {
                        is_running = false;
                        break;
                    }
                }
            }
        }
    }
    ~PipeReader() {
        cleanup();
    }
private:
    void cleanup() {
        if (pipe_read_fd != -1) close(pipe_read_fd);
        if (pipe_write_fd != -1) close(pipe_write_fd);
        if (epoll_fd != -1) close(epoll_fd);
    }
};




#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <unordered_map>
class NonBlockingServer {
private:
    int epoll_fd;
    int pipe_read_fd;
    int pipe_write_fd;
    int file_fd;
    std::unordered_map<int, std::vector<char>> client_buffers;
    static const int BUFFER_SIZE = 8192;
    bool file_fully_read;
    off_t file_offset;
public:
    NonBlockingServer() : epoll_fd(-1), pipe_read_fd(-1), pipe_write_fd(-1),
                         file_fd(-1), file_fully_read(false), file_offset(0) {
        epoll_fd = epoll_create1(0);
        if (epoll_fd == -1) {
            throw std::runtime_error("Failed to create epoll instance");
        }
    }
    // Add a file descriptor to epoll
    void addToEpoll(int fd, uint32_t events) {
        epoll_event ev;
        ev.events = events | EPOLLET;  // Edge-triggered mode
        ev.data.fd = fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
            throw std::runtime_error("Failed to add to epoll");
        }
    }
    // Set fd to non-blocking mode
    void setNonBlocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1) {
            throw std::runtime_error("Failed to get flags");
        }
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            throw std::runtime_error("Failed to set non-blocking");
        }
    }
    void setupPipeForFile(const std::string& filename) {
        // Create pipe
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            throw std::runtime_error("Failed to create pipe");
        }
        pipe_read_fd = pipefd[0];
        pipe_write_fd = pipefd[1];
        // Set both ends to non-blocking
        setNonBlocking(pipe_read_fd);
        setNonBlocking(pipe_write_fd);
        // Add read end to epoll
        addToEpoll(pipe_read_fd, EPOLLIN | EPOLLRDHUP);
        // Open file
        file_fd = open(filename.c_str(), O_RDONLY);
        if (file_fd == -1) {
            throw std::runtime_error("Failed to open file");
        }
        setNonBlocking(file_fd);
        file_offset = 0;
        file_fully_read = false;
        // Add file to epoll for write readiness
        addToEpoll(file_fd, EPOLLIN);
    }
    void addClientSocket(int client_fd) {
        setNonBlocking(client_fd);
        addToEpoll(client_fd, EPOLLIN | EPOLLOUT | EPOLLET);
        client_buffers[client_fd] = std::vector<char>();
    }
    bool processFileChunk() {
        char buffer[BUFFER_SIZE];
        while (true) {
            ssize_t bytes_read = read(file_fd, buffer, BUFFER_SIZE);
            if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    return false;  // No more data available right now
                }
                throw std::runtime_error("Failed to read from file");
            }
            if (bytes_read == 0) {
                // EOF reached
                close(pipe_write_fd);
                pipe_write_fd = -1;
                file_fully_read = true;
                return true;
            }
            // Try to write to pipe
            ssize_t bytes_written = write(pipe_write_fd, buffer, bytes_read);
            if (bytes_written == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Pipe is full, try again later
                    file_offset -= bytes_read;  // Rewind file offset
                    return false;
                }
                throw std::runtime_error("Failed to write to pipe");
            }
        }
    }
    void processEvents() {
        epoll_event events[32];
        while (true) {
            int nfds = epoll_wait(epoll_fd, events, 32, -1);
            if (nfds == -1) {
                if (errno == EINTR) continue;
                throw std::runtime_error("epoll_wait failed");
            }
            for (int n = 0; n < nfds; ++n) {
                int current_fd = events[n].data.fd;
                // Handle file reading
                if (current_fd == file_fd && !file_fully_read) {
                    if (events[n].events & EPOLLIN) {
                        processFileChunk();
                    }
                }
                // Handle pipe reading
                else if (current_fd == pipe_read_fd) {
                    if (events[n].events & EPOLLIN) {
                        handlePipeRead();
                    }
                    if (events[n].events & (EPOLLHUP | EPOLLRDHUP)) {
                        // Pipe closed, check if we need to clean up
                        if (file_fully_read) {
                            close(pipe_read_fd);
                            pipe_read_fd = -1;
                        }
                    }
                }
                // Handle client sockets
                else if (client_buffers.find(current_fd) != client_buffers.end()) {
                    if (events[n].events & EPOLLOUT) {
                        sendBufferedDataToClient(current_fd);
                    }
                }
            }
        }
    }
private:
    void handlePipeRead() {
        char buffer[BUFFER_SIZE];
        while (true) {
            ssize_t bytes_read = read(pipe_read_fd, buffer, BUFFER_SIZE);
            if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;  // No more data available right now
                }
                throw std::runtime_error("Failed to read from pipe");
            }
            if (bytes_read == 0) {
                break;  // EOF
            }
            // Buffer the data for all clients
            for (auto& [client_fd, client_buffer] : client_buffers) {
                client_buffer.insert(client_buffer.end(), buffer, buffer + bytes_read);
                // Trigger write to client
                sendBufferedDataToClient(client_fd);
            }
        }
    }
    void sendBufferedDataToClient(int client_fd) {
        auto& buffer = client_buffers[client_fd];
        while (!buffer.empty()) {
            ssize_t bytes_sent = send(client_fd, buffer.data(), buffer.size(), MSG_NOSIGNAL);
            if (bytes_sent == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Would block, try again later
                    return;
                }
                // Handle client disconnect
                closeClient(client_fd);
                return;
            }
            // Remove sent data from buffer
            buffer.erase(buffer.begin(), buffer.begin() + bytes_sent);
        }
    }
    void closeClient(int client_fd) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
        close(client_fd);
        client_buffers.erase(client_fd);
    }
    ~NonBlockingServer() {
        if (pipe_read_fd != -1) close(pipe_read_fd);
        if (pipe_write_fd != -1) close(pipe_write_fd);
        if (file_fd != -1) close(file_fd);
        if (epoll_fd != -1) close(epoll_fd);
        for (const auto& [client_fd, _] : client_buffers) {
            close(client_fd);
        }
    }
};


#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <unordered_map>
class NonBlockingServer {
private:
    int epoll_fd;
    int pipe_read_fd;
    int pipe_write_fd;
    int file_fd;
    std::unordered_map<int, std::vector<char>> client_buffers;
    static const int BUFFER_SIZE = 8192;
    bool file_fully_read;
    bool pipe_write_ready;
public:
    NonBlockingServer() : epoll_fd(-1), pipe_read_fd(-1), pipe_write_fd(-1),
                         file_fd(-1), file_fully_read(false), pipe_write_ready(true) {
        epoll_fd = epoll_create1(0);
        if (epoll_fd == -1) {
            throw std::runtime_error("Failed to create epoll instance");
        }
    }
    void setupPipeForFile(const std::string& filename) {
        // Create pipe
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            throw std::runtime_error("Failed to create pipe");
        }
        pipe_read_fd = pipefd[0];
        pipe_write_fd = pipefd[1];
        // Set both ends to non-blocking
        setNonBlocking(pipe_read_fd);
        setNonBlocking(pipe_write_fd);
        // Add ONLY the read end to epoll
        addToEpoll(pipe_read_fd, EPOLLIN | EPOLLRDHUP);
        // Open file
        file_fd = open(filename.c_str(), O_RDONLY);
        if (file_fd == -1) {
            throw std::runtime_error("Failed to open file");
        }
        // Start the first file read
        tryWriteFileChunkToPipe();
    }
    void addClientSocket(int client_fd) {
        setNonBlocking(client_fd);
        addToEpoll(client_fd, EPOLLIN | EPOLLOUT | EPOLLET);
        client_buffers[client_fd] = std::vector<char>();
    }
private:
    void addToEpoll(int fd, uint32_t events) {
        epoll_event ev;
        ev.events = events | EPOLLET;  // Edge-triggered mode
        ev.data.fd = fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
            throw std::runtime_error("Failed to add to epoll");
        }
    }
    void setNonBlocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1) {
            throw std::runtime_error("Failed to get flags");
        }
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            throw std::runtime_error("Failed to set non-blocking");
        }
    }
    void tryWriteFileChunkToPipe() {
        if (file_fully_read || !pipe_write_ready) {
            return;
        }
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(file_fd, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            ssize_t bytes_written = write(pipe_write_fd, buffer, bytes_read);
            if (bytes_written == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Pipe is full, mark it as not ready
                    pipe_write_ready = false;
                    // We'll need to rewind the file position since we couldn't write
                    lseek(file_fd, -bytes_read, SEEK_CUR);
                    return;
                }
                throw std::runtime_error("Failed to write to pipe");
            }
            // Try to write more if we successfully wrote
            tryWriteFileChunkToPipe();
        } else if (bytes_read == 0) {
            // EOF reached
            close(pipe_write_fd);
            pipe_write_fd = -1;
            file_fully_read = true;
        } else if (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            throw std::runtime_error("Failed to read from file");
        }
    }
    void handlePipeRead() {
        char buffer[BUFFER_SIZE];
        while (true) {
            ssize_t bytes_read = read(pipe_read_fd, buffer, BUFFER_SIZE);
            if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;  // No more data available right now
                }
                throw std::runtime_error("Failed to read from pipe");
            }
            if (bytes_read == 0) {
                break;  // EOF
            }
            // When we successfully read from the pipe, it means we can write more to it
            pipe_write_ready = true;
            // Buffer the data for all clients
            for (auto& [client_fd, client_buffer] : client_buffers) {
                client_buffer.insert(client_buffer.end(), buffer, buffer + bytes_read);
                // Trigger write to client
                sendBufferedDataToClient(client_fd);
            }
        }
        // If pipe is ready for more data, try writing more from file
        if (pipe_write_ready && !file_fully_read) {
            tryWriteFileChunkToPipe();
        }
    }
    void sendBufferedDataToClient(int client_fd) {
        auto& buffer = client_buffers[client_fd];
        while (!buffer.empty()) {
            ssize_t bytes_sent = send(client_fd, buffer.data(), buffer.size(), MSG_NOSIGNAL);
            if (bytes_sent == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Would block, try again later
                    return;
                }
                // Handle client disconnect
                closeClient(client_fd);
                return;
            }
            // Remove sent data from buffer
            buffer.erase(buffer.begin(), buffer.begin() + bytes_sent);
        }
    }
public:
    void processEvents() {
        epoll_event events[32];
        while (true) {
            int nfds = epoll_wait(epoll_fd, events, 32, -1);
            if (nfds == -1) {
                if (errno == EINTR) continue;
                throw std::runtime_error("epoll_wait failed");
            }
            for (int n = 0; n < nfds; ++n) {
                int current_fd = events[n].data.fd;
                // Handle pipe reading
                if (current_fd == pipe_read_fd) {
                    if (events[n].events & EPOLLIN) {
                        handlePipeRead();
                    }
                    if (events[n].events & (EPOLLHUP | EPOLLRDHUP)) {
                        if (file_fully_read) {
                            close(pipe_read_fd);
                            pipe_read_fd = -1;
                        }
                    }
                }
                // Handle client sockets
                else if (client_buffers.find(current_fd) != client_buffers.end()) {
                    if (events[n].events & EPOLLOUT) {
                        sendBufferedDataToClient(current_fd);
                    }
                }
            }
        }
    }
private:
    void closeClient(int client_fd) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
        close(client_fd);
        client_buffers.erase(client_fd);
    }
public:
    ~NonBlockingServer() {
        if (pipe_read_fd != -1) close(pipe_read_fd);
        if (pipe_write_fd != -1) close(pipe_write_fd);
        if (file_fd != -1) close(file_fd);
        if (epoll_fd != -1) close(epoll_fd);
        for (const auto& [client_fd, _] : client_buffers) {
            close(client_fd);
        }
    }
};




NUOVO












Invia messaggio dmonfrin







#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <vector>
#include <errno.h>
class FileHandler {
private:
    int pipe_fds[2];
    static const size_t BUFFER_SIZE = 4096;
    std::string accumulated_data;
    bool writing_complete;
public:
    FileHandler() : writing_complete(false) {
        if (pipe(pipe_fds) == -1) {
            throw std::runtime_error("Failed to create pipe");
        }
        // Set read end of pipe to non-blocking
        int flags = fcntl(pipe_fds[0], F_GETFL, 0);
        fcntl(pipe_fds[0], F_SETFL, flags | O_NONBLOCK);
    }
    ~FileHandler() {
        close(pipe_fds[0]);
        close(pipe_fds[1]);
    }
    // Start reading file into pipe in a separate thread
    void startFileTransfer(const std::string& filename) {
        std::thread writer_thread([this, filename]() {
            std::ifstream file(filename, std::ios::binary);
            if (!file) {
                close(pipe_fds[1]);
                return;
            }
            std::vector<char> buffer(BUFFER_SIZE);
            while (file.read(buffer.data(), BUFFER_SIZE)) {
                size_t bytes_read = file.gcount();
                size_t bytes_written = 0;
                while (bytes_written < bytes_read) {
                    ssize_t written = write(pipe_fds[1],
                                         buffer.data() + bytes_written,
                                         bytes_read - bytes_written);
                    if (written == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // Pipe is full, wait a bit
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                            continue;
                        }
                        break;
                    }
                    bytes_written += written;
                }
            }
            // Write any remaining data
            size_t bytes_read = file.gcount();
            if (bytes_read > 0) {
                write(pipe_fds[1], buffer.data(), bytes_read);
            }
            close(pipe_fds[1]);
            writing_complete = true;
        });
        writer_thread.detach();
    }
    // Get the read end of the pipe for epoll
    int getReadFd() const {
        return pipe_fds[0];
    }
    // Read available data from pipe
    std::string readPipeData() {
        std::vector<char> buffer(BUFFER_SIZE);
        std::string chunk;
        while (true) {
            ssize_t bytes_read = read(pipe_fds[0], buffer.data(), BUFFER_SIZE);
            if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // No more data available right now
                    break;
                }
                // Handle other errors
                throw std::runtime_error("Error reading from pipe");
            }
            if (bytes_read == 0) {
                // EOF reached
                break;
            }
            chunk.append(buffer.data(), bytes_read);
        }
        accumulated_data += chunk;
        return chunk;
    }
    bool isTransferComplete() const {
        return writing_complete;
    }
    const std::string& getAccumulatedData() const {
        return accumulated_data;
    }
};
// Example usage in your epoll server
class Server {
private:
    int epoll_fd;
    std::unordered_map<int, std::unique_ptr<FileHandler>> file_handlers;
public:
    void handleFileRequest(const std::string& filename, int client_fd) {
        auto file_handler = std::make_unique<FileHandler>();
        int pipe_read_fd = file_handler->getReadFd();
        // Add pipe's read end to epoll
        epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;  // Use EPOLLET for edge-triggered mode if needed
        ev.data.fd = pipe_read_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_read_fd, &ev) == -1) {
            throw std::runtime_error("Failed to add to epoll");
        }
        // Store file handler
        file_handlers[pipe_read_fd] = std::move(file_handler);
        // Start file transfer
        file_handlers[pipe_read_fd]->startFileTransfer(filename);
    }
    void handlePipeReadable(int pipe_fd) {
        auto& handler = file_handlers[pipe_fd];
        try {
            std::string chunk = handler->readPipeData();
            if (!chunk.empty()) {
                // Send chunk to client
                // Your send logic here
            }
            if (handler->isTransferComplete()) {
                // Clean up if transfer is complete
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, pipe_fd, nullptr);
                file_handlers.erase(pipe_fd);
            }
        } catch (const std::exception& e) {
            // Handle errors
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, pipe_fd, nullptr);
            file_handlers.erase(pipe_fd);
        }
    }
};











Invia messaggio dmonfrin







#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <errno.h>
class FileHandler {
private:
    static const size_t BUFFER_SIZE = 4096;
    int pipe_fds[2];
    int file_fd;
    size_t file_offset;
    bool write_complete;
    std::string accumulated_data;
public:
    FileHandler() : file_fd(-1), file_offset(0), write_complete(false) {
        if (pipe(pipe_fds) == -1) {
            throw std::runtime_error("Failed to create pipe");
        }
        // Set both ends of pipe to non-blocking
        for (int i = 0; i < 2; ++i) {
            int flags = fcntl(pipe_fds[i], F_GETFL, 0);
            fcntl(pipe_fds[i], F_SETFL, flags | O_NONBLOCK);
        }
    }
    ~FileHandler() {
        if (file_fd != -1) close(file_fd);
        close(pipe_fds[0]);
        close(pipe_fds[1]);
    }
    bool openFile(const std::string& filename) {
        file_fd = open(filename.c_str(), O_RDONLY | O_NONBLOCK);
        return file_fd != -1;
    }
    // Get file descriptor for writing to pipe
    int getWriteFd() const {
        return pipe_fds[1];
    }
    // Get file descriptor for reading from pipe
    int getReadFd() const {
        return pipe_fds[0];
    }
    // Try to write more file data to pipe
    bool writeMoreData() {
        if (write_complete) return false;
        std::vector<char> buffer(BUFFER_SIZE);
        ssize_t bytes_read = read(file_fd, buffer.data(), BUFFER_SIZE);
        if (bytes_read > 0) {
            size_t bytes_written = 0;
            while (bytes_written < static_cast<size_t>(bytes_read)) {
                ssize_t written = write(pipe_fds[1],
                                     buffer.data() + bytes_written,
                                     bytes_read - bytes_written);
                if (written == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // Pipe is full, we'll try again later
                        lseek(file_fd, file_offset + bytes_written, SEEK_SET);
                        return true;
                    }
                    // Real error occurred
                    return false;
                }
                bytes_written += written;
            }
            file_offset += bytes_written;
            return true;
        }
        else if (bytes_read == 0) {
            // EOF reached
            write_complete = true;
            close(pipe_fds[1]); // Close write end of pipe
            return false;
        }
        // Error reading file
        return false;
    }
    // Read available data from pipe
    std::string readPipeData() {
        std::vector<char> buffer(BUFFER_SIZE);
        std::string chunk;
        while (true) {
            ssize_t bytes_read = read(pipe_fds[0], buffer.data(), BUFFER_SIZE);
            if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                throw std::runtime_error("Error reading from pipe");
            }
            if (bytes_read == 0) {
                break;
            }
            chunk.append(buffer.data(), bytes_read);
        }
        accumulated_data += chunk;
        return chunk;
    }
    bool isComplete() const {
        return write_complete;
    }
    const std::string& getAccumulatedData() const {
        return accumulated_data;
    }
};
class Server {
private:
    int epoll_fd;
    std::map<int, std::unique_ptr<FileHandler>> file_handlers;
public:
    void handleFileRequest(const std::string& filename, int client_fd) {
        auto file_handler = std::make_unique<FileHandler>();
        if (!file_handler->openFile(filename)) {
            // Handle file open error
            return;
        }
        int pipe_read_fd = file_handler->getReadFd();
        int pipe_write_fd = file_handler->getWriteFd();
        // Add pipe's read end to epoll
        epoll_event ev_read;
        ev_read.events = EPOLLIN;  // Level-triggered
        ev_read.data.fd = pipe_read_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_read_fd, &ev_read) == -1) {
            throw std::runtime_error("Failed to add read pipe to epoll");
        }
        // Add pipe's write end to epoll
        epoll_event ev_write;
        ev_write.events = EPOLLOUT;  // Level-triggered
        ev_write.data.fd = pipe_write_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_write_fd, &ev_write) == -1) {
            throw std::runtime_error("Failed to add write pipe to epoll");
        }
        // Store file handler
        file_handlers[pipe_read_fd] = std::move(file_handler);
    }
    void handleEvents(epoll_event& event) {
        if (event.events & EPOLLIN) {
            handlePipeReadable(event.data.fd);
        }
        if (event.events & EPOLLOUT) {
            handlePipeWritable(event.data.fd);
        }
    }
private:
    void handlePipeReadable(int pipe_fd) {
        auto& handler = file_handlers[pipe_fd];
        try {
            std::string chunk = handler->readPipeData();
            if (!chunk.empty()) {
                // Send chunk to client
                // Your send logic here
            }
            if (handler->isComplete()) {
                // Clean up if transfer is complete
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, pipe_fd, nullptr);
                file_handlers.erase(pipe_fd);
            }
        } catch (const std::exception& e) {
            // Handle errors
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, pipe_fd, nullptr);
            file_handlers.erase(pipe_fd);
        }
    }
    void handlePipeWritable(int pipe_fd) {
        auto it = file_handlers.begin();
        while (it != file_handlers.end()) {
            if (it->second->getWriteFd() == pipe_fd) {
                if (!it->second->writeMoreData()) {
                    // Remove write fd from epoll when done or on error
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, pipe_fd, nullptr);
                }
                break;
            }
            ++it;
        }
    }
};




NUOVO












Invia messaggio dmonfrin








#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <string>
class CGIHandler {
private:
    int epoll_fd;
    int pipe_read_fd;
    int pipe_write_fd;
    pid_t cgi_pid;
    std::vector<char> request_body;
    size_t bytes_written;
    bool write_completed;
public:
    CGIHandler(const std::vector<char>& body)
        : request_body(body), bytes_written(0), write_completed(false) {
        epoll_fd = epoll_create1(0);
    }
    bool setup_pipe_and_fork() {
        int pipe_fds[2];
        if (pipe(pipe_fds) == -1) {
            return false;
        }
        pipe_read_fd = pipe_fds[0];
        pipe_write_fd = pipe_fds[1];
        // Set pipe write end to non-blocking
        int flags = fcntl(pipe_write_fd, F_GETFL, 0);
        fcntl(pipe_write_fd, F_SETFL, flags | O_NONBLOCK);
        cgi_pid = fork();
        if (cgi_pid == -1) {
            close(pipe_read_fd);
            close(pipe_write_fd);
            return false;
        }
        if (cgi_pid == 0) {
            // Child process
            close(pipe_write_fd);  // Close write end
            dup2(pipe_read_fd, STDIN_FILENO);
            close(pipe_read_fd);
            // Setup environment and execute CGI script
            execve("./cgi-script", nullptr, nullptr);
            exit(1);
        }
        // Parent process
        close(pipe_read_fd);  // Close read end
        // Add pipe write end to epoll
        epoll_event ev;
        ev.events = EPOLLOUT | EPOLLHUP;
        ev.data.fd = pipe_write_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_write_fd, &ev) == -1) {
            return false;
        }
        return true;
    }
    bool handle_epoll_events() {
        epoll_event events[1];
        int nfds = epoll_wait(epoll_fd, events, 1, -1);
        if (nfds == -1) {
            return false;
        }
        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLHUP) {
                // Handle hangup - this is normal after write end is closed
                if (write_completed) {
                    // All data was written, this is expected
                    cleanup();
                    return true;
                } else {
                    // Unexpected hangup before writing completed
                    cleanup();
                    return false;
                }
            }
            if (events[i].events & EPOLLOUT) {
                // Write more data to the pipe
                ssize_t bytes = write(pipe_write_fd,
                                   request_body.data() + bytes_written,
                                   request_body.size() - bytes_written);
                if (bytes > 0) {
                    bytes_written += bytes;
                    if (bytes_written == request_body.size()) {
                        // All data written, close write end
                        close(pipe_write_fd);
                        write_completed = true;
                    }
                } else if (bytes == -1) {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        cleanup();
                        return false;
                    }
                }
            }
        }
        return true;
    }
    void cleanup() {
        close(pipe_write_fd);
        close(epoll_fd);
        // You might want to waitpid() for the child here
        // depending on your needs
    }
    ~CGIHandler() {
        cleanup();
    }
};




