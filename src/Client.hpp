#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <fstream>
#include <ostream>
#include <sstream>

const int BUFFER_SIZE = 1024;

class Client {
private:
	int socket_fd;
	std::string server_ip;
	int server_port;
public:
	Client(int socket_fd, std::string server_ip, int server_port) : socket_fd(socket_fd), server_ip(server_ip), server_port(server_port) {}
	~Client();
	void cleanup() {
		if (socket_fd != -1) {
			close(socket_fd);
			std::cout << "Server socket closed.\n";
		};
	void send
}