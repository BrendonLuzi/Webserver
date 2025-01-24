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

//   Config(std::string &path);
  Config();
  ~Config();


  void tokenizethefile();
  ConfigNode parseConfigFile(const std::string &filename);
  std::string trim(const std::string &str);
  void printConfig(const ConfigNode &node, int depth);
      // Getter methods
    std::string getWorkers() const { return workers; }
    std::string getHost() const { return host; }
    std::string getCgi() const { return cgi; }
    std::string getRootDir() const { return rootDir; }
    std::string getErrorPage() const { return errorPage; }

 private:
    int fd;
     // Specific attributes for storing parsed values
    std::string workers;
    std::string host;
    std::string cgi;
    std::string rootDir;
    std::string errorPage;


};

#endif