# UNDER CONSTRUCTION
# Webserv
Webserv is a virtual webserver coded in C++. 

## Compiling
**Linux**
To compile on linux, run "make" at the root of the repository

```bash	
make
```

**Non-Linux**
To compile on other systems, use Docker and the provided Dockerfile. If you don't have Docker yet, you will have to install and run it first. To compile the project via docker, run (after having started up Docker):

```bash
docker build -t container .
```

## Running
This webserver takes one argument: a configuration file. Sample configuration files can be found in the "config" repository.

**Linux**
To run the project on Linux, execute the program with the desired configuration via the command line, like so:

```bash
./webserver path_to_config_file
```

**Non-linux**
To run the project on a non-Linux system, run the following command in your command line.

```bash
docker run --rm -it container /bin/bash
```

This will run the container in interactive mode, opening a new shell on the command line.
If you want to run the webserver with a different configuration file, you can easily edit the Dockerfile (last line, see comment in Dockerfile)