#include "Route.hpp"

Route::Route() {}

Route::Route(Config::ConfigNode &parent, Config::ConfigNode &location) {
	std::array<std::string, 11> possible_directives = {"root", "index", "allow_methods", "types", "error_page", "client_max_body_size", "redirect", "autoindex", "cgi", "access_log", "error_log"};

	// Set the uri
	directives["uri"] = location.directives["uri"];

	// Set the other directives
	for (int i = 0; i < 11; i++) {
		// if directive value is not empty string in location block, set it
		if (location.directives[possible_directives[i]] != "") {
			directives[possible_directives[i]] = location.directives[possible_directives[i]];
		}
		// else if directive value is not empty string in parent block, set it and save it in location block
		else if (parent.directives[possible_directives[i]] != "") {
			directives[possible_directives[i]] = parent.directives[possible_directives[i]];
			location.directives[possible_directives[i]] = parent.directives[possible_directives[i]];
		}
	}
}

Route::~Route() {}

void Route::addDirective(const std::string& directive, const std::string& value) {
	directives[directive] = value;
}

std::string Route::getDirective(const std::string& directive) const {
	std::map<std::string, std::string>::const_iterator it = directives.find(directive);
	if (it != directives.end()) {
		return it->second;
	}
	return "";
}

std::ostream& operator<<(std::ostream& os, const Route& route) {
	std::array<std::string, 11> possible_directives = {"root", "index", "allow_methods", "type", "error_page", "client_max_body_size", "redirect", "autoindex", "cgi", "access_log", "error_log"};
	for (int i = 0; i < 11; i++) {
		os << possible_directives[i] << ": " << route.getDirective(possible_directives[i]) << std::endl;
	}
	return os;
}