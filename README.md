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

## Benchmark

Benchmark for uploading a test file (PDF, 11.3 MB) with different numbers of concurrent connections, 200 times.

| Server         | Concurrency | Req/s | Time(s) | Success | Errors |
| -------------- | ----------- | ----- | ------- | ------- | ------ |
| C++ server     | 1           | 59.70 | 3.35    | 200     | 0      |
| C++ server     | 10          | 94.54 | 2.12    | 200     | 0      |
| C++ server     | 50          | 95.50 | 2.09    | 200     | 0      |
| C++ server     | 100         | 97.32 | 2.06    | 200     | 0      |
| Express server | 1           | 54.98 | 3.64    | 200     | 0      |
| Express server | 10          | 72.10 | 2.77    | 200     | 0      |
| Express server | 50          | 61.26 | 3.26    | 200     | 0      |
| Express server | 100         | 54.71 | 3.66    | 115     | 85     |
| Flask server   | 1           | 37.22 | 5.37    | 200     | 0      |
| Flask server   | 10          | 32.74 | 6.11    | 200     | 0      |
| Flask server   | 50          | 35.12 | 5.69    | 200     | 0      |
| Flask server   | 100         | 35.22 | 5.68    | 200     | 0      |

Tested on a MacBook Pro 2020 M2, 16GB
