# Simple HTTP Server

A lightweight C++ HTTP server that supports file uploads, downloads, and serving static webpages.  
Designed for experimenting with networking concepts and basic web functionality.

---

## Compile the Project
To build the server, run:

```bash
g++ -o build/server server.cpp src/header_type_converter.cpp src/header_type.cpp src/response_type_converter.cpp src/header_converter.cpp src/client_handler.cpp -Iinclude -pthread -std=c++20
```

## Project Structure
 - storage/ — contains uploaded files
 - public/ — contains static files and webpages served by the server
 - src/ — source code files
 - include/ — header files
 - build/ — compiled binary output

## Capabilities
 - Handles GET and POST requests
 - Supports persistent connections
 - Echo endpoint - returns the contents of a GET request in the response body
 - Files endpoint - supports file upload (POST) and file download (GET)
 - Serves complete webpages from the public folder

TODO:
 - Compression support
