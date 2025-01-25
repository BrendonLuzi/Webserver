#include "Server.hpp"

Server::Server(Config::ConfigNode &server) {
	// For each port in the configuration, create a socket and set it to listen on that port
	std::vector<std::string> listen_directive = split(server.directives["listen"], " ");

	for (std::vector<std::string>::iterator port = listen_directive.begin(); port != listen_directive.end(); ++port) {
		// Create socket
		int server_fd;
		server_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (server_fd < 0) {
			std::cerr << "Error: Socket creation failed\n";
			exit(1);
		}

		// Set socket to non-blocking
		// fcntl(server_fd, F_SETFL, O_NONBLOCK);

		// Bind socket to port #
		struct sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;

		// If the value contains a colon, use the part after the colon
		int port_num;
		if (port->find(":") != std::string::npos) {
			port_num = std::stoi(port->substr(port->find_last_of(":") + 1));
		} else {
			port_num = std::stoi(*port);
		}
		address.sin_port = htons(port_num);

		// Bind the socket to the address
		if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
			std::cerr << "Error: Bind failed\n";
			close(server_fd);
			exit(1);
		}

		// Listen for incoming connections
		if (listen(server_fd, 10) == -1) {
			std::cerr << "Error: Listen failed\n";
			close(server_fd);
			exit(1);
		}

		// Add server socket to sockets
		pollfd server_socket = { server_fd, POLLIN, 0 };
		_sockets.push_back(server_socket);
		_server_fd.push_back(server_fd);

		std::cout << MAGENTA << "Server with fd " << server_fd << " listening on port "<< port_num <<" ..." << RESET << std::endl;
	}
}

Server::~Server() {
	// Close all open sockets
	for (int i = 0; i < _sockets.size(); i++) {
		if (_sockets[i].fd != -1)
		{
			close(_sockets[i].fd);
			std::cout << YELLOW << "Socket " << _sockets[i].fd << " closed" << RESET << std::endl;
		}
	}
}

void Server::accept_connection(int server_fd) {
	// Open new socket and accept connection
	int new_socket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
		std::cerr << "Error: Accept failed\n";
		close(server_fd);
		exit(1);
	}

	// Set new socket to non-blocking
	// fcntl(new_socket, F_SETFL, O_NONBLOCK);
	
	// Add new socket to _sockets
	pollfd client_socket = { new_socket, POLLIN, 0};
	_sockets.push_back(client_socket);

	std::cout << GREEN << "New connection, socket fd is " << new_socket << RESET << std::endl;
}

void Server::read_request(int i) {
	// Read request header
	char buffer[BUFFER_SIZE] = {0};
	ssize_t bytes_received;
	std::string request_data = "";
	std::string header_end = "\r\n\r\n";

	// Read until the end of the header is found
	size_t header_end_pos = request_data.find(header_end);
	while (header_end_pos == std::string::npos) {
		bytes_received = recv(_sockets[i].fd, buffer, BUFFER_SIZE, 0);
		request_data.append(buffer, bytes_received);
		header_end_pos = request_data.find(header_end);
	}

	// Find content length and read request body
	std::string headers = request_data.substr(0, header_end_pos);
	size_t content_length_pos = headers.find("Content-Length: ");

	if (content_length_pos != std::string::npos) {
		size_t content_length_end_pos = headers.find("\r\n", content_length_pos);
		std::string content_length_str = headers.substr(content_length_pos + 16, content_length_end_pos - (content_length_pos + 16));
		int content_length = std::stoi(content_length_str);

		std::cout << YELLOW << headers << std::endl;
		std::cout << GREEN << request_data << RESET << std::endl;


		while (request_data.size() < header_end_pos + 4 + content_length) {
			bytes_received = recv(_sockets[i].fd, buffer, BUFFER_SIZE, 0);
			if (bytes_received == -1) {
				std::cerr << "Error: Receive failed\n";
				close(_sockets[i].fd);
				// close(_server_fd);
				exit (EXIT_FAILURE);
			}
			request_data.append(buffer, bytes_received);
			std::cout << "Request data size: " << request_data.size() << std::endl;
		}
	}

	//print request
	std::cout<<BLUE<<"New Request from "<<_sockets[i].fd<<RESET<<std::endl;
	std::cout<<CYAN<<request_data<<RESET<<std::endl;	

	// Parse and store the request
	HttpRequest request(request_data);
	_requests[_sockets[i].fd] = request;

	// Set the socket to wait for the response
	_sockets[i].events = POLLOUT;
}

void Server::write_response(int i) {
	// Prepare response
	Response response = router->route(_requests[_sockets[i].fd]);
	
	std::cout<<BLUE<<"Response: "<<response.get_status_code()<<RESET<<std::endl;
	
	// Send response
	std::string response_str = response.to_string();
	send(_sockets[i].fd, response_str.c_str(), response_str.size(), 0);
	std::cout << response_str << std::endl;

	// Erase the request
	_requests.erase(_sockets[i].fd);

	// Check if connection is keep-alive
	bool keep_alive = _requests[_sockets[i].fd].get_connection() == "keep-alive";

	// If connection is not keep-alive, close the socket
	if (!keep_alive) {
		std::cout << RED << "Connection closed for socket " <<_sockets[i].fd << RESET << std::endl;
		close(_sockets[i].fd);
		_sockets.erase(_sockets.begin() + i);
	}
	else {
		std::cout << YELLOW << "Connection kept alive for socket " << _sockets[i].fd << RESET << std::endl;
		_sockets[i].events = POLLIN;
	}
}

void Server::check_sockets()  {
	// Check all sockets in the server for events in non-blocking mode
	poll(_sockets.data(), _sockets.size(), 0);

	// Loop through all sockets in the server
	for (int i = 0; i < _sockets.size(); i++) {
		// If POLLHUP or POLLERR events occur, close the socket			
		if (_sockets[i].revents & POLLHUP || _sockets[i].revents & POLLERR) {
			close(_sockets[i].fd);
			_sockets.erase(_sockets.begin() + i);
		}
		// If POLLIN event occurs, read the request
		else if (_sockets[i].revents & POLLIN) {
			// If a server socket has an event, accept the connection
			if (std::find(_server_fd.begin(), _server_fd.end(), _sockets[i].fd) != _server_fd.end())
				accept_connection(_sockets[i].fd);
			// If a client socket has an event, read the request
			else
				read_request(i);
		}
		else if (_sockets[i].revents & POLLOUT)
			write_response(i);
	}
}
