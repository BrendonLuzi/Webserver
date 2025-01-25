#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <fstream>
#include <ostream>
#include <sstream>
#include "HttpRequest.hpp"
#include "Route.hpp"


class Response {
private:
    int status_code;
    std::map<std::string, std::string> headers;
    std::string body;

public:
	Response() {}
    Response(int status_code, std::map<std::string, std::string> headers, std::string body)
        : status_code(status_code), headers(headers), body(body) {
        // Automatically add Content-Length header if missing
        if (headers.find("Content-Length") == headers.end() && !body.empty()) {
            std::stringstream ss;
            ss << body.size();
            headers["Content-Length"] = ss.str();
        }
    }

    int get_status_code() const {
        return status_code;
    }

    const std::map<std::string, std::string>& get_headers() const {
        return headers;
    }

    const std::string& get_body() const {
        return body;
    }

    std::string to_string() const {
        std::string response = "HTTP/1.1 ";

        std::stringstream ss;
        ss << status_code;
        response += ss.str();

        response += "\r\n";

        // Add headers
        for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
            response += it->first + ": " + it->second + "\r\n";
        }

        // Add body if it's not empty
        if (!body.empty()) {
            response += "\r\n" + body;
        }

        return response;
    }
};

Response handleGetRequest(const HttpRequest& request, const Route& route);
Response handlePostRequest(const HttpRequest& request, const Route& route);
Response handleDeleteRequest(const HttpRequest& request, const Route& route);
Response handleCgiRequest(const HttpRequest& request, const Route& route);
Response handleCgiRequest(const HttpRequest& request, const Route& route, const std::string& query);

Response executeCgi(const std::string& interpreter, const std::string& filePath, const std::string& query_string);

#endif // RESPONSE_HPP
