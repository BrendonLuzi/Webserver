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
		HttpRequest() {}

		HttpRequest(const std::string& request);

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
	
		std::string get_connection() const;
};

std::ostream& operator<<(std::ostream& os, const HttpRequest& request);

#endif