#include "Server.hpp"

Server::Server(Config::ConfigNode &server) {
	// Create socket
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0) {
		std::cerr << "Error: Socket creation failed\n";
		exit(1);
	}
	// Set socket to non-blocking
	// fcntl(_server_fd, F_SETFL, O_NONBLOCK);

	// Bind socket to port #
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	// set port as the listen directive. if the value contains a colon, use the port after the colon
	int port;
	if (server.directives["listen"].find(":") != std::string::npos) {
		port = std::stoi(server.directives["listen"].substr(server.directives["listen"].find(":") + 1));
	} else {
		port = std::stoi(server.directives["listen"]);
	}
	address.sin_port = htons(port);
	if (bind(_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		std::cerr << "Error: Bind failed\n";
		close(_server_fd);
		exit(1);
	}

	// Listen for incoming connections
	if (listen(_server_fd, 10) == -1) {
		std::cerr << "Error: Listen failed\n";
		close(_server_fd);
		exit(1);
	}

	// Add server socket to sockets
	pollfd server_socket = { _server_fd, POLLIN, 0 };
	_sockets.push_back(server_socket);

	// fd_set read_fds; // Declare an fd_set structure
	// FD_ZERO(&read_fds); // Clear the set
	// FD_SET(_server_fd, &read_fds); // Add _server_fd to the set
	_port = port;
	std::cout << MAGENTA << "Server listening on port "<< port <<"..." << RESET << std::endl;
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

void Server::accept_connection() {
	// Open new socket and accept connection
	int new_socket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	if ((new_socket = accept(_server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
		std::cerr << "Error: Accept failed\n";
		close(_server_fd);
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
	std::cout << RED << "YOU KNOW IT" << RESET << std::endl;
	// Read request header
	char buffer[BUFFER_SIZE] = {0};
	ssize_t bytes_received;
	std::string request_data = "";
	std::string header_end = "\r\n\r\n";
	size_t header_end_pos = request_data.find(header_end);
	while (header_end_pos == std::string::npos) {
		bytes_received = recv(_sockets[i].fd, buffer, BUFFER_SIZE, 0);
		request_data.append(buffer, bytes_received);
		std::cout << "Received " << bytes_received << " bytes" << std::endl;
		header_end_pos = request_data.find(header_end);
	}
	std::cout << RED << "YOU KNOW IT 2" << RESET << std::endl;

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
				close(_server_fd);
				exit (EXIT_FAILURE);
			}
			request_data.append(buffer, bytes_received);
			std::cout << "Request data size: " << request_data.size() << std::endl;
		}
	}
	std::cout << RED << "YOU KNOW IT 3" << RESET << std::endl;

	if (bytes_received == -1) {
		std::cerr << "Error: Receive failed\n";
		close(_sockets[i].fd);
		close(_server_fd);
		exit (EXIT_FAILURE);
	}

	//print request
	std::cout<<BLUE<<"New Request from "<<_sockets[i].fd<<RESET<<std::endl;
	std::cout<<CYAN<<request_data<<RESET<<std::endl;	

	// Parse and store the request
	HttpRequest request(request_data);
	_requests[_sockets[i].fd] = request;

	// Prepare response
	Response response = router->route(request);
	
	std::cout<<BLUE<<"Response: "<<response.get_status_code()<<RESET<<std::endl;
	
	// Save response to _responses and set socket to POLLOUT
	_responses[_sockets[i].fd] = response;
	_sockets[i].events = POLLOUT;
}

void Server::write_response(int i) {
	// Send response
	std::string response_str = _responses[_sockets[i].fd].to_string();
	send(_sockets[i].fd, response_str.c_str(), response_str.size(), 0);
	std::cout << response_str << std::endl;

	// Check if connection is keep-alive
	bool keep_alive = _requests[_sockets[i].fd].get_connection() == "keep-alive";

	// Erase request and response
	_requests.erase(_sockets[i].fd);
	_responses.erase(_sockets[i].fd);

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
	// Check all sockets for events
	poll(_sockets.data(), _sockets.size(), 0);

	// Loop through all sockets
	for (int i = 0; i < _sockets.size(); i++) {
		// If POLLHUP or POLLERR events occur, close the socket			
		if (_sockets[i].revents & POLLHUP || _sockets[i].revents & POLLERR) {
			close(_sockets[i].fd);
			_sockets.erase(_sockets.begin() + i);
		}
		// If POLLIN event occurs, read the request
		else if (_sockets[i].revents & POLLIN) {
			// If the server socket has an event, accept the connection
			if (_sockets[i].fd == _server_fd)
				accept_connection();
			// If a client socket has an event, read the request
			else
				read_request(i);
		}
		else if (_sockets[i].revents & POLLOUT)
			write_response(i);
	}
}
