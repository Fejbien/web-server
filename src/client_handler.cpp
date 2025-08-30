#include "client_handler.h"

const std::string ClientHandler::HTTP_STANDARD = "HTTP/1.1 ";
const std::string ClientHandler::HTTP_RESPOND_OK = "200 OK";
const std::string ClientHandler::HTTP_RESPOND_NOT_FOUND = "404 Not Found";
const std::string ClientHandler::HTTP_RESPOND_ENDING_SINGLE = "\r\n";
const std::string ClientHandler::HTTP_RESPOND_ENDING_DOUBLE = "\r\n\r\n";

void ClientHandler::handle_get_request(const request &req, int client_fd)
{
    std::string sendingString = ClientHandler::HTTP_STANDARD;
    if (req.path == "/" || req.path == "/index.html")
    {
        sendingString += ClientHandler::HTTP_RESPOND_OK + ClientHandler::HTTP_RESPOND_ENDING_DOUBLE;
    }
    else if (req.path.substr(0, 5).compare("/echo") == 0)
    {
        sendingString += ClientHandler::HTTP_RESPOND_OK + ClientHandler::HTTP_RESPOND_ENDING_SINGLE;
        sendingString += "Content-Type: text/plain" + ClientHandler::HTTP_RESPOND_ENDING_SINGLE;
        sendingString += "Content-Length: " + std::to_string(req.path.substr(6).size()) + ClientHandler::HTTP_RESPOND_ENDING_DOUBLE;
        sendingString += req.path.substr(6);
    }
    else if (req.path.substr(0, 12).compare("/user-agent") == 0)
    {
        std::string user_agent = HeaderConverter::get_header_value(req.buffer, HeaderType::USER_AGENT);

        sendingString += HTTP_RESPOND_OK + HTTP_RESPOND_ENDING_SINGLE;
        sendingString += "Content-Type: text/plain" + HTTP_RESPOND_ENDING_SINGLE;
        sendingString += "Content-Length: " + std::to_string(user_agent.size()) + HTTP_RESPOND_ENDING_DOUBLE;
        sendingString += user_agent;
    }
    else if (req.path.substr(0, 6).compare("/files") == 0)
    {
        std::string file_path = req.path.substr(7);
        // std::cout << "Requested file: " << file_path << "\n";

        // Prevent directory traversal attacks
        if (file_path.find("..") != std::string::npos)
        {
            sendingString += HTTP_RESPOND_NOT_FOUND + HTTP_RESPOND_ENDING_DOUBLE;
        }
        else
        {
            FILE *file = fopen(file_path.c_str(), "rb");
            if (file)
            {
                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                fseek(file, 0, SEEK_SET);

                sendingString += HTTP_RESPOND_OK + HTTP_RESPOND_ENDING_SINGLE;
                sendingString += "Content-Type: application/octet-stream" + HTTP_RESPOND_ENDING_SINGLE;
                sendingString += "Content-Length: " + std::to_string(file_size) + HTTP_RESPOND_ENDING_DOUBLE;

                char *file_buffer = new char[file_size];
                fread(file_buffer, 1, file_size, file);
                sendingString.append(file_buffer, file_size);
                delete[] file_buffer;
                fclose(file);
            }
            else
            {
                sendingString += HTTP_RESPOND_NOT_FOUND + HTTP_RESPOND_ENDING_DOUBLE;
            }
        }
    }
    else
    {
        sendingString += HTTP_RESPOND_NOT_FOUND + HTTP_RESPOND_ENDING_DOUBLE;
    }

    send(client_fd, sendingString.c_str(), sendingString.size(), 0);
}

void ClientHandler::handle_client(int client_fd)
{
    char buffer[1024];
    int is_error = recv(client_fd, buffer, sizeof(buffer), 0);
    if (is_error < 0)
    {
        std::cerr << "recv failed\n";
        return;
    }

    request req;
    req.buffer = buffer;
    req.method = HeaderConverter::recognize_header_request(buffer);
    req.path = HeaderConverter::get_header_value(buffer, req.method);

    switch (req.method)
    {
    case HeaderType::GET:
        ClientHandler::handle_get_request(req, client_fd);
        break;
    default:
        std::cerr << "Unsupported HTTP method\n";
        break;
    }

    close(client_fd);
}