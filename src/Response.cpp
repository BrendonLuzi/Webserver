#include "Response.hpp"
#include <iostream>
#include "helper_functions.hpp"

Response handleGetRequest(const HttpRequest& request, const Route& route) {
	std::map<std::string, std::string> headers;
	std::string uri = request.get_uri();
	std::string filePath = route.getDirective("root") + uri;

	// if the location allows CGI, handle CGI
	if (route.getDirective("cgi") != "") {
		// if the extension is in cgi, handle CGI
		std::string extension = " " + uri.substr(uri.find_last_of(".") + 1) + " ";
		if (route.getDirective("cgi").find(extension) != std::string::npos) {
			// handle CGI
			std::cout << "CGI is on" << std::endl;
			// return handleCgiRequest(request, route);
			return Response(200, headers, "CGI is on");
		}
	}

	// if the uri is a directory and autoindex is off, append the index file
	if (uri.back() == '/') {
		if (route.getDirective("autoindex") == "off") {
			filePath += route.getDirective("index");
		}
		else {
			// generate the autoindex page
			std::cout << "Autoindex is on" << std::endl;
		}
	}

	// If the file exists, read it and return a 200 OK response
	if (isFileGood(filePath, "GET")) {
		std::string body = readFile(filePath);

		if (!body.empty()) {
			// Set the Content-Type based on the file extension
			std::string extension = uri.substr(uri.find_last_of(".") + 1);
			std::string contentType = getContentType(extension);

			headers["Cache-Control"] = "public, max-age=31536000";  // Cache for one year
			headers["Content-Type"] = contentType;
			headers["Content-Length"] = std::to_string(body.size());
			return Response(200, headers, body);
		}
		else
		{
			headers["Cache-Control"] = "no-store";
			headers["Content-Type"] = "text/plain";
			headers["Content-Length"] = "9";
			return Response(500, headers, "Internal Server Error");
		}
	}

	headers["Cache-Control"] = "no-store";
	headers["Content-Type"] = "text/plain";
	headers["Content-Length"] = "9";
	return Response(404, headers, "Not Found");
}

Response handlePostRequest(const HttpRequest& request, const Route& route) {
	std::map<std::string, std::string> headers;
	std::string body = request.get_body();
	std::string uri = request.get_uri();
	std::string filePath = route.getDirective("root") + uri;

	// Check if CGI is enabled
	if (route.getDirective("cgi") != "") {
		// Check if the extension is in the list of CGI extensions
		std::string extension = " " + uri.substr(uri.find_last_of(".") + 1) + " ";
		if (route.getDirective("cgi").find(extension) != std::string::npos) {
			// Handle CGI
			std::cout << "CGI is on" << std::endl;
			// return handleCgiRequest(request, route);
			return Response(200, headers, "CGI is on");
		}
	}

	// create the directory for the file if it doesn't exist
	std::vector<std::string> directories = split(filePath, "/");
	std::string currentPath = "";
	for (std::vector<std::string>::iterator it = directories.begin(); it != directories.end(); ++it) {
		currentPath += *it + "/";
		// if directory exists, continue
		if (isFileGood(currentPath, "POST")) {
			continue;
		}
		else if (mkdir(currentPath.c_str(), 0777) == -1) {
			headers["Content-Type"] = "text/plain";
			headers["Content-Length"] = "9";
			return Response(500, headers, "Server Error");
		}
	}

	// get boundary from content-type
	std::string contentType = request.get_headers()["content-type"];
	std::string boundary = get_boundary(contentType);

	// Split the body into parts using the boundary
    std::vector<std::string> parts = split_by_boundary(body, boundary);

    bool file_uploaded = false;

    // Process each part (either form field or file)
	for (std::vector<std::string>::iterator it = parts.begin(); it != parts.end(); ++it) {
    	const std::string& part = *it;
        if (part.find("Content-Disposition: form-data;") != std::string::npos) {
            // Check if it's a file part by looking for the "filename" field
            if (part.find("filename=\"") != std::string::npos) {
				// If the file exists, return a 409 Conflict response
				if (isFileGood(filePath + "/" + get_filename(part), "POST")) {
					headers["Content-Type"] = "text/plain";
					headers["Content-Length"] = "9";
					return Response(409, headers, "Conflict");
				}
                save_uploaded_file(part, filePath);
                file_uploaded = true;
            } else {
                // Process form field (not implemented here)
                std::cout << "Form field: " << part << std::endl;
            }
        }
    }

	if (file_uploaded) {
		headers["Content-Type"] = "text/plain";
		headers["Content-Length"] = "20";
		return Response(201, headers, "File created successfully");
	}

	headers["Content-Type"] = "text/plain";
	headers["Content-Length"] = "9";
	return Response(500, headers, "Server Error");
}

Response handleDeleteRequest(const HttpRequest& request, const Route& route) {
	std::map<std::string, std::string> headers;
	std::string uri = request.get_uri();
	std::string filePath = route.getDirective("root") + uri;

	// If the file exists, delete it and return a 200 OK response
	if (isFileGood(filePath, "DELETE")) {
		if (remove(filePath.c_str()) == 0) {
			headers["Content-Type"] = "text/plain";
			headers["Content-Length"] = "11";
			return Response(200, headers, "File deleted");
		}
		else
		{
			headers["Content-Type"] = "text/plain";
			headers["Content-Length"] = "9";
			return Response(500, headers, "Server Error");
		}
	}

	headers["Content-Type"] = "text/plain";
	headers["Content-Length"] = "9";
	return Response(404, headers, "Not Found");
}

// Response handleCgiRequest(const HttpRequest& request, const Route& route) {
// 	std::map<std::string, std::string> headers;
// 	std::string uri = request.get_uri();
// 	std::string filePath = route.getDirective("root") + uri;

// 	std::string filename
// }
