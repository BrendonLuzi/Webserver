#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <map>
#include <string>
#include <iostream>
#include "Response.hpp"
#include "HttpRequest.hpp"
#include "Route.hpp"
#include "Config.hpp"
#include "helper_functions.hpp"

class Router {
	private:
		std::map<std::string, Route> routes;

	public:
		Router();
		Router(Config::ConfigNode &config, Config::ConfigNode &server);

		void getLocations(Config::ConfigNode &node);
		void add_route(const std::string& uri, const Route& route);
		Response route(const HttpRequest& request) const;
		Route findRoute(const HttpRequest& request) const;

		// getter for routes
		std::map<std::string, Route> getRoutes() const { return routes; }
};

#endif