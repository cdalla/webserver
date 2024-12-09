# Specify container OS.
FROM ubuntu:latest

EXPOSE 8080

# Install packages & software.
RUN apt-get update && apt-get install -y build-essential curl python3


# Set the working directory inside the container
WORKDIR /webserver

# Copy source files to working directory
COPY . .

# Compile code
RUN ["make"]

# Run the webserver. The second string is the argument: change this if you want to change the configuration file
# CMD ["./webserv", "configurations/naomi.conf"]
CMD ["./webserv"]


