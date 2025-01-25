#ifndef ROUTE_HPP
#define ROUTE_HPP

#include <string>
#include <vector>
#include <map>
#include "Config.hpp"
#include "helper_functions.hpp"
#include <array>

class Route {
public:
    Route();
	Route(Config::ConfigNode &parent, Config::ConfigNode &location);
	~Route();

	void addDirective(const std::string& directive, const std::string& value);
	std::string getDirective(const std::string& directive) const;
	
private:
	std::map<std::string, std::string> directives;
};

std::ostream& operator<<(std::ostream& os, const Route& route);

#endif // ROUTE_HPP