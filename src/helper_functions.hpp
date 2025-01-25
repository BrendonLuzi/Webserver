#ifndef HELPER_FUNCTIONS_HPP
#define HELPER_FUNCTIONS_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <map>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include "HttpRequest.hpp"


// Colors for console output
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define RESET "\033[0m"

bool isFileGood(const std::string& filename, std::string method);
bool isDirectory(const std::string& filename);
std::string get_filename(const std::string& part);
std::string readFile(const std::string& filename);
std::string getContentType(const std::string& extension);
void signal_handler(int signal);
void cleanup();
std::string get_boundary(const std::string& content_type);
std::vector<std::string> split_by_boundary(const std::string& body, const std::string& boundary);
void save_uploaded_file(const std::string& part, std::string &filePath);
std::vector<std::string> split(const std::string& str, const std::string& delim);

#endif // HELPER_FUNCTIONS_HPP