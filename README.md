# Simple web server

C++ 20 was used

## Compile the project:

g++ -o build/server server.cpp src/header_type_converter.cpp src/header_type.cpp src/response_type_converter.cpp src/header_converter.cpp src/client_handler.cpp -Iinclude -pthread -std=c++20

## Current project

Server hosts a simple website that is allowing to upload files to a server and allows to download any file that is currently stored in storage folder. Websites that are exposed from server are being store in public folder

## Current capabilities

- Handles GET and POST requests
- Persistent connection
- Echo endpoint which returns the latter of get reuqest as the body
- Files endpoint supporting GET and POST requests
- Support for serving full webpages

### Add later:

- Compression
