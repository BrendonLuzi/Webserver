#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <iostream>


class HttpRequest {
	private:
		std::string method;
		std::string uri;
		std::string version;
		std::map<std::string, std::string> headers;
		std::string body;
	public:
		HttpRequest() {
			method = "";
			uri = "";
			version = "";
		}

		HttpRequest(const std::string& request) {
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

				// Normalize header name to lowercase
				std::transform(name.begin(), name.end(), name.begin(), ::tolower);
				headers[name] = value;
				std::cout << "Header: " << name << " Value: " << value << std::endl;
			}

        	// Parse the body if Content-Length is specified
			if (headers.count("content-length") > 0) {
				size_t content_length = std::stoi(headers["content-length"]);
				body.resize(content_length);
				request_stream.read(&body[0], content_length);
        	}
    	}

    std::string get_method() const { return method; }
    std::string get_uri() const { return uri; }
    std::string get_version() const { return version; }
    std::map<std::string, std::string> get_headers() const { return headers; }
    std::string get_body() const { return body; }
    std::string get_host() const {
        if (headers.count("host") > 0) {
            return headers.at("host");  // returns the value of the Host header
        }
        throw std::runtime_error("Host header missing");
    }
	
	std::string get_connection() const {
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
};

std::ostream& operator<<(std::ostream& os, const HttpRequest& request);

#endif