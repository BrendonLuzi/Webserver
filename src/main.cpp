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
			// Close all open sockets and connections
			// ...

			// Save port as a string
			std::string port = std::to_string((*server)->get_port());
			// Delete the server
			delete *server;

			std::cout << MAGENTA << "Server listening on port " << port << " terminated." << RESET << std::endl;
		}
	}
    exit(signal);
}

void server_loop(std::vector<Server *> servers) {
	std::vector<pollfd> poll_sockets;
	
	while (true) {
		// For each server, check all sockets for events
		for (std::vector<Server *>::iterator server = servers.begin(); server != servers.end(); ++server) {
			(*server)->check_sockets();
		}
	}
}

int main() {

	// Register signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

	// Read Configuration
	Config *config = new Config();
	config->content=config->parseConfigFile("./webserv.conf");
	// config->printConfig(config->content, 0);

	Config::ConfigNode http_config = config->content.blocks["http"][0];

	// Create a router for each server in the configuration
	for (std::vector<Config::ConfigNode>::iterator server_config = http_config.blocks["server"].begin(); server_config != http_config.blocks["server"].end(); ++server_config) {
		Router *router = new Router(http_config, *server_config);
		
		// Start the servers following the configuration
		Server *server = new Server(*server_config);
		server->router = router;
		servers.push_back(server);
	}

	// Run the servers
	server_loop(servers);
	
	// // Start the servers
	// server = new Server(5505, -1);
	// server->router = router;
	// server->run();
	// delete server;

    return 0;
}
