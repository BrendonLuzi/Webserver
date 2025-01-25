#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <fstream>
#include <ostream>
#include <sstream>
#include <map>
#include "HttpRequest.hpp"
#include "helper_functions.hpp"
#include "Server.hpp"
#include "Router.hpp"
#include "Config.hpp"

std::vector<Server *> servers;

void signal_handler(int signal) {
	for (std::vector<Server *>::iterator server = servers.begin(); server != servers.end(); ++server) {
		if (*server != NULL) {
			// Delete the server
			delete *server;

			std::cout << MAGENTA << "Server terminated." << RESET << std::endl;
		}
	}
    exit(signal);
}

void server_loop(std::vector<Server *> servers) {
	while (true) {
		// For each server, check all sockets for events
		for (std::vector<Server *>::iterator server = servers.begin(); server != servers.end(); ++server) {
			(*server)->check_sockets();
		}
	}
}

int main(int argc, char *argv[]) {
	// Check if the program was run with one argument
	if (argc != 2) {
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		exit(1);
	}

	// Register signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

	// Read Configuration
	Config *config = new Config();

	if (isFileGood(argv[1], "")) {
		config->content=config->parseConfigFile(argv[1]);
	} else {
		std::cerr << "Error: Configuration file not found." << std::endl;
		exit(1);
	}

	// Get the http configuration
	Config::ConfigNode http_config = config->content.blocks["http"][0];

	// For each server in the configuration, create a server and its respective router
	for (std::vector<Config::ConfigNode>::iterator server_config = http_config.blocks["server"].begin(); server_config != http_config.blocks["server"].end(); ++server_config) {
		Server *server = new Server(*server_config);
		server->router = new Router(http_config, *server_config);
		servers.push_back(server);
	}

	// Run the servers
	server_loop(servers);

    return 0;
}
