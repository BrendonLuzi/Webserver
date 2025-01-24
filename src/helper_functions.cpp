#include "helper_functions.hpp"
#include "Server.hpp"

std::vector<std::string> split(const std::string& str, const std::string& delim) {
	std::vector<std::string> tokens;
	size_t start = 0, end = 0;
	while ((end = str.find(delim, start)) != std::string::npos) {
		tokens.push_back(str.substr(start, end - start));
		start = end + delim.length();
	}
	tokens.push_back(str.substr(start));
	return tokens;
}

bool isFileGood(const std::string& filename, std::string method) {
	struct stat buffer;
	// check if the file exists, 
	if (stat(filename.c_str(), &buffer) != 0) {
		return false;
	}
	// if it's a get request, check if it's readable
	if (method == "GET" && access(filename.c_str(), R_OK) != 0) {
		return false;
	// }
	// // if it's a post request, check if it's writable
	// if (method == "POST" && access(filename.c_str(), W_OK) != 0) {
	// 	return false;
	}
	return true;
}

std::string readFile(const std::string& filename) {
    std::ifstream _file(filename.c_str());
    std::string content;

    if (_file.is_open()) {
        std::string line;
        while (std::getline(_file, line)) {
            content += line + "\n";
        }
        _file.close();
    } else {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }

    return content;
}

std::string getContentType(const std::string& extension) {
    if (extension == "jpg" || extension == "jpeg") {
        return "image/jpeg";
    } else if (extension == "png") {
        return "image/png";
    } else if (extension == "html") {
        return "text/html";
    } else if (extension == "css") {
        return "text/css";
    } else if (extension == "js") {
        return "application/javascript";
    } else {
        return "text/plain";
    }
}

std::string get_boundary(const std::string& content_type) {
    std::string boundary_prefix = "boundary=";
    size_t boundary_pos = content_type.find(boundary_prefix);

	std::string boundary = content_type.substr(boundary_pos + boundary_prefix.length());

    if (boundary_pos != std::string::npos) {
		//trim boundary	from /r/n
		boundary = boundary.substr(0, boundary.find_last_not_of("\r\n") + 1);
		return boundary;	
    }
    return "";
}

// Function to split the body into parts using the boundary
std::vector<std::string> split_by_boundary(const std::string& body, const std::string& boundary) {
    std::vector<std::string> parts;
    std::string boundary_with_prefix = "--" + boundary;
    std::string end_boundary = boundary_with_prefix + "--";
    size_t part_start = 0;
    size_t part_end = 0;

    while (true) {
        part_start = body.find(boundary_with_prefix, part_end);
        if (part_start == std::string::npos) {
            break;
        }
        part_start += boundary_with_prefix.length();

        // Skip initial CRLF after boundary
        if (body.substr(part_start, 2) == "\r\n") {
            part_start += 2;
        }

        part_end = body.find(boundary_with_prefix, part_start);
        if (part_end == std::string::npos) {
            // Check for the end boundary
            part_end = body.find(end_boundary, part_start);
            if (part_end == std::string::npos) {
                // Handle the last part
                parts.push_back(body.substr(part_start));
                break;
            } else {
                // Handle the last part before the end boundary
                size_t content_end = part_end;
                if (body.substr(content_end - 2, 2) == "\r\n") {
                    content_end -= 2;
                }
                parts.push_back(body.substr(part_start, content_end - part_start));
                break;
            }
        }

        // Skip the trailing CRLF before the next boundary
        size_t content_end = part_end;
        if (body.substr(content_end - 2, 2) == "\r\n") {
            content_end -= 2;
        }

        parts.push_back(body.substr(part_start, content_end - part_start));
        part_end += boundary_with_prefix.length();
    }
    return parts;
}

std::string get_filename(const std::string& part) {
	std::string filename_prefix = "filename=\"";
	size_t filename_pos = part.find(filename_prefix);
	if (filename_pos != std::string::npos) {
		size_t filename_start = filename_pos + filename_prefix.length();
		size_t filename_end = part.find("\"", filename_start);
		return part.substr(filename_start, filename_end - filename_start);
	}
	return "";
}

void save_uploaded_file(const std::string& part, std::string &filePath) {
	std::string filename = get_filename(part);
	std::string file_content = part.substr(part.find("\r\n\r\n") + 4);

	// print file Path in yellow
	//std::cout << YELLOW << "File Path: " << filePath << RESET << std::endl;
	// print filename in magenta
	//std::cout << MAGENTA << "Filename: " << filename << RESET << std::endl;
	// print file content in cyan
	//std::cout << CYAN << "File Content: " << file_content << RESET << std::endl;

	// Save the content in the file, overwriting if it already exists
	std::ofstream file(filePath + "/" + filename, std::ios::binary);
	file << file_content;
	file.close();
}
