#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <stack>
#include <sstream>
#include <string>
#include <fcntl.h>

class Config {
	public:
		std::string path;

		struct ConfigNode {
			std::map<std::string, std::string> directives; // Key-value pairs (e.g., "workers 8;")
			std::map<std::string, std::vector<ConfigNode> > blocks; // Nested blocks (e.g., "server")
			};

		ConfigNode content;

		Config();
		~Config();


		ConfigNode parseConfigFile(const std::string &filename);
		std::string trim(const std::string &str); // Should move to helper_functions.hpp
		void printConfig(const ConfigNode &node, int depth);
};

#endif