#include "Response.hpp"
#include <iostream>
#include "helper_functions.hpp"

Response handleGetRequest(const HttpRequest& request, const Route& route) {
	std::map<std::string, std::string> headers;
	std::string uri = request.get_uri();
	std::string filePath = route.getDirective("root") + uri;

	// if the location allows CGI, handle CGI
	if (route.getDirective("cgi") != "") {
		// split the uri to get the query string
		std::string query = "";
		std::string extension = "";
		if (uri.find("?") != std::string::npos) {
			query = uri.substr(uri.find("?") + 1);
			extension = uri.substr(0, uri.find("?"));
			extension = extension.substr(extension.find_last_of(".") + 1) + " ";
		}
		else
			extension = uri.substr(uri.find_last_of(".") + 1) + " ";
		
		if (route.getDirective("cgi").find(extension) != std::string::npos) {
			// handle CGI
			return handleCgiRequest(request, route, query);
		}
	}

	// if the uri is a directory and autoindex is off, append the index file
	if (uri.back() == '/' && isDirectory(filePath)) {
		if (route.getDirective("autoindex") == "off") {
			filePath += route.getDirective("index");
		}
		else {
			// generate the autoindex page by executin the index.py script
			return executeCgi("../public/cgi-bin/index.py", "/usr/bin/python3", filePath);
		}
	}

	// If the file exists, read it and return a 200 OK response
	if (isFileGood(filePath, "GET")) {
		std::string body = readFile(filePath);

		if (!body.empty()) {
			// Set the Content-Type based on the file extension
			std::string extension = uri.substr(uri.find_last_of(".") + 1);
			std::string contentType = getContentType(extension);

			headers["Cache-Control"] = "public, max-age=31536000";  // Cache for one year
			headers["Content-Type"] = contentType;
			headers["Content-Length"] = std::to_string(body.size());
			return Response(200, headers, body);
		}
		else
		{
			headers["Cache-Control"] = "no-store";
			headers["Content-Type"] = "text/plain";
			headers["Content-Length"] = "9";
			return Response(500, headers, "Internal Server Error");
		}
	}

	headers["Cache-Control"] = "no-store";
	headers["Content-Type"] = "text/plain";
	headers["Content-Length"] = "9";
	return Response(404, headers, "Not Found");
}

Response handlePostRequest(const HttpRequest& request, const Route& route) {
	std::map<std::string, std::string> headers;
	std::string body = request.get_body();
	std::string uri = request.get_uri();
	std::string filePath = route.getDirective("root") + uri;

	// Check if CGI is enabled
	if (route.getDirective("cgi") != "") {
		// Check if the extension is in the list of CGI extensions
		std::string extension = " " + uri.substr(uri.find_last_of(".") + 1) + " ";
		if (route.getDirective("cgi").find(extension) != std::string::npos) {
			// Handle CGI
			std::cout << "CGI is on" << std::endl;
			// return handleCgiRequest(request, route);
			return Response(200, headers, "CGI is on");
		}
	}

	// create the directory for the file if it doesn't exist
	std::vector<std::string> directories = split(filePath, "/");
	std::string currentPath = "";
	for (std::vector<std::string>::iterator it = directories.begin(); it != directories.end(); ++it) {
		currentPath += *it + "/";
		// if directory exists, continue
		if (isFileGood(currentPath, "POST")) {
			continue;
		}
		else if (mkdir(currentPath.c_str(), 0777) == -1) {
			headers["Content-Type"] = "text/plain";
			headers["Content-Length"] = "9";
			return Response(500, headers, "Server Error");
		}
	}

	// get boundary from content-type
	std::string contentType = request.get_headers()["content-type"];
	std::string boundary = get_boundary(contentType);

	// Split the body into parts using the boundary
    std::vector<std::string> parts = split_by_boundary(body, boundary);

    bool file_uploaded = false;

    // Process each part (either form field or file)
	for (std::vector<std::string>::iterator it = parts.begin(); it != parts.end(); ++it) {
    	const std::string& part = *it;
        if (part.find("Content-Disposition: form-data;") != std::string::npos) {
            // Check if it's a file part by looking for the "filename" field
            if (part.find("filename=\"") != std::string::npos) {
				// If the file exists, return a 409 Conflict response
				if (isFileGood(filePath + "/" + get_filename(part), "POST")) {
					headers["Content-Type"] = "text/plain";
					headers["Content-Length"] = "9";
					return Response(409, headers, "Conflict");
				}
                save_uploaded_file(part, filePath);
                file_uploaded = true;
            } else {
                // Process form field (not implemented here)
                std::cout << "Form field: " << part << std::endl;
            }
        }
    }

	if (file_uploaded) {
		headers["Content-Type"] = "text/plain";
		headers["Content-Length"] = "20";
		return Response(201, headers, "File created successfully");
	}

	headers["Content-Type"] = "text/plain";
	headers["Content-Length"] = "9";
	return Response(500, headers, "Server Error");
}

Response handleDeleteRequest(const HttpRequest& request, const Route& route) {
	std::map<std::string, std::string> headers;
	std::string uri = request.get_uri();
	std::string filePath = route.getDirective("root") + uri;

	// If the file exists, delete it and return a 200 OK response
	if (isFileGood(filePath, "DELETE")) {
		if (remove(filePath.c_str()) == 0) {
			headers["Content-Type"] = "text/plain";
			headers["Content-Length"] = "11";
			return Response(200, headers, "File deleted");
		}
		else
		{
			headers["Content-Type"] = "text/plain";
			headers["Content-Length"] = "9";
			return Response(500, headers, "Server Error");
		}
	}

	headers["Content-Type"] = "text/plain";
	headers["Content-Length"] = "9";
	return Response(404, headers, "Not Found");
}

Response handleCgiRequest(const HttpRequest& request, const Route& route) {
	std::map<std::string, std::string> headers;
	std::string uri = request.get_uri();
	std::string filePath = route.getDirective("root") + uri;
	std::string interpreter = "";

	// 

	// Find extension of the file
	std::string extension = uri.substr(uri.find_last_of(".") + 1);

	std::vector<std::string> cgiDirectives = split(route.getDirective("cgi"), " ");
	// Find the path to the interpreter after the extension
	for (std::vector<std::string>::iterator it = cgiDirectives.begin(); it != cgiDirectives.end(); ++it) {
		if (it->find(extension) != std::string::npos && it + 1 != cgiDirectives.end()) {
			interpreter = *(it + 1);
			break;
		}
	}

	// print the info
	std::cout << "Interpreter: " << interpreter << std::endl;
	std::cout << "File Path: " << filePath << std::endl;

	// If the interpreter is not found, or it doesn't exist, return a 500 Internal Server Error response
	if (interpreter == "" || !isFileGood(interpreter, "CGI")) {
		headers["Content-Type"] = "text/plain";
		headers["Content-Length"] = "9";
		return Response(500, headers, "Server Error");
	}

	// If the file does not exist, return a 404 Not Found response
	if (!isFileGood(filePath, "CGI")) {
		headers["Content-Type"] = "text/plain";
		headers["Content-Length"] = "9";
		return Response(404, headers, "Not Found");
	}

	return executeCgi(filePath, interpreter, "");
}

Response handleCgiRequest(const HttpRequest& request, const Route& route, const std::string& query) {
	std::map<std::string, std::string> headers;
	std::string uri = request.get_uri();
	std::string filePath = route.getDirective("root") + uri;
	std::string interpreter = "";

	if (query != "") {
		uri = uri.substr(0, uri.find("?"));
	}

	// Find extension of the file
	std::string extension = uri.substr(uri.find_last_of(".") + 1);

	std::vector<std::string> cgiDirectives = split(route.getDirective("cgi"), " ");
	// Find the path to the interpreter after the extension
	for (std::vector<std::string>::iterator it = cgiDirectives.begin(); it != cgiDirectives.end(); ++it) {
		if (it->find(extension) != std::string::npos && it + 1 != cgiDirectives.end()) {
			interpreter = *(it + 1);
			break;
		}
	}

	// print the info
	std::cout << "Interpreter: " << interpreter << std::endl;
	std::cout << "File Path: " << filePath << std::endl;

	// If the interpreter is not found, or it doesn't exist, return a 500 Internal Server Error response
	if (interpreter == "" || !isFileGood(interpreter, "CGI")) {
		headers["Content-Type"] = "text/plain";
		headers["Content-Length"] = "9";
		return Response(500, headers, "Server Error");
	}

	// If the file does not exist, return a 404 Not Found response
	if (!isFileGood(filePath, "CGI")) {
		headers["Content-Type"] = "text/plain";
		headers["Content-Length"] = "9";
		return Response(404, headers, "Not Found");
	}

	return executeCgi(filePath, interpreter, query);
}

Response executeCgi(const std::string& filePath, const std::string& interpreter, const std::string& query_string) {
	std::map<std::string, std::string> headers;
	std::string body = "";
	std::string response = "";

	// Create a pipe to read the output of the CGI script
	int pipefd[2];
	if (pipe(pipefd) == -1) {
		headers["Content-Type"] = "text/plain";
		headers["Content-Length"] = "9";
		return Response(500, headers, "Server Error");
	}

	// Fork the process
	pid_t pid = fork();
	if (pid == -1) {
		headers["Content-Type"] = "text/plain";
		headers["Content-Length"] = "9";
		return Response(500, headers, "Server Error");
	}

	if (pid == 0) {
		// Child process
		// Redirect stdout to the write end of the pipe
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);

		// Set environment variables
		setenv("REQUEST_METHOD", "GET", 1);
		setenv("SCRIPT_FILENAME", filePath.c_str(), 1);
		setenv("QUERY_STRING", query_string.c_str(), 1);

		// Execute the CGI script
		execl(interpreter.c_str(), interpreter.c_str(), filePath.c_str(), query_string.c_str(), NULL);
		exit(1);
	} else {
		// Parent process
		// Close the write end of the pipe
		close(pipefd[1]);

		// Read the output of the CGI script
		char buffer[4096];
		ssize_t bytesRead;
		while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
			body.append(buffer, bytesRead);
		}

		// Wait for the child process to finish
		int status;
		waitpid(pid, &status, 0);

		// Close the read end of the pipe
		close(pipefd[0]);

		// Parse the response
		std::istringstream response_stream(body);
		std::string line;
		// Parse the headers
		while (std::getline(response_stream, line) && !line.empty() && line != "\r") {
			size_t colon_pos = line.find(':');
			if (colon_pos == std::string::npos) {
				throw std::runtime_error("Malformed header line: " + line);
			}

			// Extract name and value
			std::string name = line.substr(0, colon_pos);
			std::string value = line.substr(colon_pos + 1);

			// Trim whitespace
			name.erase(0, name.find_first_not_of(" \t"));
			name.erase(name.find_last_not_of(" \t") + 1);
			value.erase(0, value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t") + 1);

			// Normalize header name to lowercase (maybe don't, it's confusing later on)
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
			headers[name] = value;
		}

		// Read the rest of the body
		while (std::getline(response_stream, line)) {
			response += line + "\n";
		}
		
		// Check if the response contains a status code
		if (headers.find("status") != headers.end()) {
			std::string status = headers["status"];
			int status_code = std::stoi(status.substr(0, status.find(" ")));
			
			// Remove the status from the headers and body
			headers.erase("status");

			return Response(status_code, headers, response);
		}

		// Return error
		headers["Content-Type"] = "text/plain";
		headers["Content-Length"] = "9";
		return Response(500, headers, "Server Error");
	}
}