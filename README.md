# Web server

## Compile the project:

g++ -o build/server server.cpp src/header_type.cpp src/header_converter.cpp src/client_handler.cpp -Iinclude -pthread

## Current capabilities

- Handles GET and POST reuqest
- Persistent connection
- Echo endpoint which returns the latter of get reuqest as the body
- Files GET and POST request, GET downloaded a file if exists and POST creates or updates a file
- Support for serving html, css and js

### Add later:

- Compression
- Fun idea: make some curd app within
