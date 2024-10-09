# Specify container OS.
FROM ubuntu:latest

# Install packages & software.
RUN apt-get update && apt-get install -y build-essential

# Set the working directory inside the container
WORKDIR /webserv

# Copy source files to working directory
COPY . /webserv

CMD ls -laR

# # Compile code
# CMD ["make"]

# Run code
# CMD ["webserver", "config/test.conf"]

