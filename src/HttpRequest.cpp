#include "HttpRequest.hpp"

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