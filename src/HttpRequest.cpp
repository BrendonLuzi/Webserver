#include "HttpRequest.hpp"

HttpRequest::HttpRequest(const std::string& request) {
	std::istringstream request_stream(request);
	std::string line;

	// Parse the request line
	if (!std::getline(request_stream, line) || line.empty()) {
		throw std::runtime_error("Malformed request: Missing request line");
	}
	std::istringstream line_stream(line);

	if (!(line_stream >> method >> uri >> version)) {
		throw std::runtime_error("Malformed request line: " + line);
	}

	// Parse the headers
	while (std::getline(request_stream, line) && !line.empty() && line != "\r") {
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

	// Parse the body if Content-Length is specified
	if (headers.count("content-length") > 0) {
		size_t content_length = std::stoi(headers["content-length"]);
		body.resize(content_length);
		request_stream.read(&body[0], content_length);
	}
}

std::string HttpRequest::get_connection() const  {
	std::map<std::string, std::string>::const_iterator it = headers.find("Connection");
	if (it != headers.end()) {
		//return it->second as a string	and trim edges of a header value
		std::string connection = it->second;
		connection.erase(0, connection.find_first_not_of(" \t\r\n"));
		connection.erase(connection.find_last_not_of(" \t\r\n") + 1);

		return connection;
	}
	return "";
}

std::ostream& operator<<(std::ostream& os, const HttpRequest& request) {
    os << "Method: " << request.get_method() << "\n"
       << "URI: " << request.get_uri() << "\n"
       << "Version: " << request.get_version() << "\n";

    std::map<std::string, std::string> headers = request.get_headers();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        os << it->first << ": " << it->second << "\n";
    }

    os << "\nBody: " << request.get_body() << "\n";

    return os;
}