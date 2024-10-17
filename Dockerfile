# Specify container OS.
FROM ubuntu:latest

# Install packages & software.
RUN apt-get update && apt-get install -y build-essential

# Set the working directory inside the container
WORKDIR /webserv

# Copy source files to working directory
COPY . /webserv

RUN ls -laR

# Compile code
RUN ["make"]

# Run the webserver. The second string is the argument: change this if you want to change the configuration file
# RUN ["./webserver", "config/test.conf"]
