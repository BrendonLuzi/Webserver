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
#include <vector>
#include <algorithm>

const int BUFFER_SIZE = 1024;

class Server {
	std::vector<int> _server_fd;
	std::vector<pollfd> _sockets;
	std::map<int, HttpRequest> _requests;
	std::map<int, std::string> _buffers;
	std::map<int, bool> _continue; 

public:
	Router *router;
	Server(Config::ConfigNode &server);
	~Server();

	void check_sockets();
	void accept_connection(int server_fd);
	void read_request(int i);
	void write_response(int i);
	std::vector<pollfd> get_sockets() const { return _sockets; }
};

#endif
