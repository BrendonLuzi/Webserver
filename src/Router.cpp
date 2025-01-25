#include "Router.hpp"

Router::Router() {}

Router::Router(Config::ConfigNode &config, Config::ConfigNode &server) {
	// Fill empty directives in server with config values or default values
	std::array<std::string, 11> possible_directives = {"root", "index", "allow_methods", "types", "error_page", "client_max_body_size", "redirect", "autoindex", "cgi", "access_log", "error_log"};
	std::map<std::string, std::string> default_values = {
		{"root", "html"},
		{"index", "index.html"},
		{"allow_methods", "GET HEAD POST"},
		{"types", ""},
		{"error_page", ""},
		{"client_max_body_size", "1m"},
		{"redirect", ""},
		{"autoindex", "off"},
		{"cgi", ""},
		{"access_log", "log/access.log"},
		{"error_log", "log/error.log"}
	};

	for (int i = 0; i < 11; i++) {
		if (server.directives[possible_directives[i]] == "") {
			if (config.directives[possible_directives[i]] != "") {
				server.directives[possible_directives[i]] = config.directives[possible_directives[i]];
			} else {
				server.directives[possible_directives[i]] = default_values[possible_directives[i]];
			}
		}
	}

	// Create a route for each location block
	getLocations(server);

	// Create default route for / with server directives if not set
	if (routes.find("/") == routes.end()) {
		Route *defaultRoute = new Route();
		for (int i = 0; i < 11; i++) {
			defaultRoute->addDirective(possible_directives[i], server.directives[possible_directives[i]]);
		}
		routes["/"] = *defaultRoute;
	}
}

// Function to create a route for each location recursively
void Router::getLocations(Config::ConfigNode &node) {
	// Create a route for each location block
	for (std::map<std::string, std::vector<Config::ConfigNode> >::iterator location = node.blocks.begin();
		location != node.blocks.end(); ++location) {
		for (size_t i = 0; i < location->second.size(); ++i) {
			// if uri is not set, define it
			if (location->second[i].directives["uri"] == "") {
				// set the uri to the key of the location block, skipping the "location " part
				location->second[i].directives["uri"] = location->first.substr(9);
			}

			Route route(node, location->second[i]);
			
			// std::cout << "_____________________________________________________" << std::endl;
			// std::cout << MAGENTA << "Route: " << route.getDirective("uri") << std::endl << CYAN << route << RESET << std::endl << std::endl;
			
			// add the route to the routes map
			routes[route.getDirective("uri")] = route;

			// if the block contains nested blocks, call the function recursively
			if (location->second.size() > 0) {
				getLocations(location->second[i]);
			}
		}
	}
}

Route Router::findRoute(const HttpRequest &request) const {
	const std::string& method = request.get_method();
    const std::string& uri = request.get_uri();

    // Go through routes object in the router, and try to find a match
	for (std::map<std::string, Route>::const_iterator route = routes.begin(); route != routes.end(); ++route) {
		if (uri == route->second.getDirective("uri")) {
			// return matching route
			return route->second;
		}
	}

	// Check for prefix match
	std::string prefix = uri;
	while (prefix != "/" && prefix[0] == '/') {
		prefix = prefix.substr(0, prefix.find_last_of("/"));
		for (std::map<std::string, Route>::const_iterator route = routes.begin(); route != routes.end(); ++route) {
			if (prefix == route->second.getDirective("uri")) {
				return route->second;
			}
		}
	}
	
	// No match found, return dafault route
	return routes.at("/");
}

Response Router::route(const HttpRequest& request) const {
	Route route = findRoute(request);

	// Check if the method is allowed
	std::string allow_methods = route.getDirective("allow_methods");
	if (allow_methods.find(request.get_method()) == std::string::npos) {
		// Method not allowed, return 405
		return Response(405, std::map<std::string, std::string>(), "Method Not Allowed");
	}

	// Check if the method is GET
	if (request.get_method() == "GET") {
		// handle GET request
		return handleGetRequest(request, route);
	}
	else if (request.get_method() == "POST") {
		// handle POST request
		return handlePostRequest(request, route);
	}
	else if (request.get_method() == "DELETE") {
		// handle DELETE request
		return handleDeleteRequest(request, route);
	}
	else { // Should not happen, maybe remove
		// Method not implemented, return 501
		return Response(501, std::map<std::string, std::string>(), "Not Implemented");
	}

    // No match found, return 404
    return Response(404, std::map<std::string, std::string>(), "Not Found");
}
