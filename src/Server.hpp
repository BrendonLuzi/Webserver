#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
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
#include "Response.hpp"
#include "helper_functions.hpp"
#include <unordered_map>
#include <map>
#include "Router.hpp"
#include <vector>
#include <poll.h>
#include <fcntl.h>
#include "Config.hpp"

const int BUFFER_SIZE = 1024;

class Server {
	int _port;
	int _server_fd;
	std::vector<pollfd> _sockets;
	std::map<int, Response> _responses;
	std::map<int, HttpRequest> _requests;
public:
	Router *router;
	Server(Config::ConfigNode &server);
	~Server();
	void check_sockets();
	void accept_connection();
	void read_request(int i);
	void write_response(int i);
	int get_port() const { return _port; }
	std::vector<pollfd> get_sockets() const { return _sockets; }
};

// struct ServerGroup {
//     std::unordered_map<std::string, Server*> servers;

//     ServerGroup(Server* defaultServer) {
//         servers["default"] = defaultServer;
//     }

// 	~ServerGroup() {
// 		std::unordered_map<std::string, Server*>::iterator it;
// 		for (it = servers.begin(); it != servers.end(); ++it) {
// 			// it->first gives you the key
// 			// it->second gives you the value
// 			delete it->second;
// 		}
// 	}
// };

// std::unordered_map<int, ServerGroup*> serverGroup;

// void initializeServers() {
//     // Create a default server for port 80
// 	if (serverGroup.find(80) == serverGroup.end())
//     	serverGroup[80] = new ServerGroup(new Server(80, socket(AF_INET, SOCK_STREAM, 0)));
// 	// Add a named server for port 80
// 	if (serverGroup[80]->servers.find("myserver.com") == serverGroup[80]->servers.end())
// 		serverGroup[80]->servers["myserver.com"] = new Server(80, socket(AF_INET, SOCK_STREAM, 0));
// 	else
// 		std::cerr << "Error: Server already exists\n";
// }

#endif
