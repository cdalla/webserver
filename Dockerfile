# Specify container OS.
FROM ubuntu:latest

EXPOSE 8080

# Install packages & software.
RUN apt-get update && apt-get install -y build-essential curl


# Set the working directory inside the container
WORKDIR /webserv

# Copy source files to working directory
COPY . .

# Compile code
RUN ["make"]

# Run the webserver. The second string is the argument: change this if you want to change the configuration file
CMD ["./webserver", "configurations/sample.conf"]

