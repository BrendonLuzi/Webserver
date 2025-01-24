#include "Config.hpp"
#include "Route.hpp"


// Config::Config(std::string &path) : path(path){
//   fd = open(path.c_str(), O_RDONLY);
//   if (fd < 0)
//     throw "could not open configuration file";
// };
Config::Config() {  
};

Config::~Config() {
	// close(fd);
};

// Helper function to trim whitespace
std::string Config::trim(const std::string &str) {
    size_t first = str.find_first_not_of(" \t");
    size_t last = str.find_last_not_of(" \t");
     if (first == std::string::npos || last == std::string::npos) {
        // If no non-whitespace characters are found, return an empty string
        return "";
    } else {
        // Otherwise, return the substring from first to last (inclusive)
        return str.substr(first, last - first + 1);
    }
}

// A structure to hold configuration data
// struct ConfigNode {
//     std::map<std::string, std::string> directives; // Key-value pairs (e.g., "workers 8;")
//     std::map<std::string, std::vector<ConfigNode> > blocks; // Nested blocks (e.g., "server")
// };

// Function to parse the configuration file iteratively
Config::ConfigNode Config::parseConfigFile(const std::string &filename) {
    Config::ConfigNode root;
    std::ifstream file(filename.c_str());

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open config file: " << filename << std::endl;
        return root;
    }

    std::string line;
    std::stack<Config::ConfigNode*> nodeStack; // Stack to track the current block
        // Specific attributes for storing parsed values
    std::string workers;
    std::string host;
    std::string cgi;
    std::string rootDir;
    std::string errorPage;
    nodeStack.push(&root);

    while (std::getline(file, line)) {
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Handle block closing brace
        if (line == "}") {
            if (!nodeStack.empty()) {
                nodeStack.pop(); // Move up a level in the hierarchy
            }
            continue;
        }

        // Check if this line starts a new block
        size_t blockStart = line.find('{');
        if (blockStart != std::string::npos) {
            std::string blockName = trim(line.substr(0, blockStart));
            Config::ConfigNode childNode;

            // Add the new block to the current node
            Config::ConfigNode* parentNode = nodeStack.top();
            parentNode->blocks[blockName].push_back(childNode);

            // Push the new block onto the stack
            nodeStack.push(&parentNode->blocks[blockName].back());
            continue;
        }

        // Handle a single directive (key-value pair)
        size_t delimiter = line.find(';');
        if (delimiter != std::string::npos) {
            std::string directive = trim(line.substr(0, delimiter));
            size_t space = directive.find(' ');

            if (space != std::string::npos) {
                std::string key = trim(directive.substr(0, space));
                std::string value = trim(directive.substr(space + 1));
                if (key == "workers") {
                        workers = value;
                }
                else if (key == "host") {
                    host = value;
                } 
                else if (key == "cgi") {
                    cgi = value;
                } 
                else if (key == "root") {
                    rootDir = value;
                } 
                else if (key == "error_page") {
                    errorPage = value;
                }
                nodeStack.top()->directives[key] = value;

            } else {
                nodeStack.top()->directives[directive] = ""; // For directives without values
            }
        }
    }

    // Store specific values in the Config object, if its not exist put default values
    if ( workers == "" ){
        this->workers = "Whatever it supposed to be";
    } else {
        this->workers = workers;
    }
    if ( host == "" ){
          this->host = "localhost";
    } else {
        this->host = host;      
    }
    if ( cgi == ""){
        this->cgi = "Whatever it supposed to be";
    } else {
        this->cgi = cgi;
    }
    if ( rootDir == "" ) {
        this->rootDir = "Whatever it supposed to be";
    } else {
        this->rootDir = rootDir;
    }
    if ( errorPage == "" ) {
        this->errorPage = "Whatever it supposed to be" ;
    } else {
        this->errorPage = errorPage;
    }
    file.close();
    return root;
}

// Function to print the configuration for debugging
void Config::printConfig(const Config::ConfigNode &node, int depth = 0) {
    std::string indent(depth * 2, ' ');

    // Print directives
    for (std::map<std::string, std::string>::const_iterator it = node.directives.begin();
         it != node.directives.end(); ++it) {
        std::cout << indent << it->first << " = " << it->second << ";" << std::endl;
    }

    // Print blocks
    for (std::map<std::string, std::vector<Config::ConfigNode> >::const_iterator it = node.blocks.begin();
         it != node.blocks.end(); ++it) {
        for (size_t i = 0; i < it->second.size(); ++i) {
            std::cout << indent << it->first << std::endl;
            printConfig(it->second[i], depth + 1);
            std::cout << indent << std::endl;
        }
    }
}
