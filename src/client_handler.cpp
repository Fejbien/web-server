#include "client_handler.h"

const std::string ClientHandler::HTTP_STANDARD = "HTTP/1.1 ";
const std::string ClientHandler::HTTP_RESPOND_OK = "200 OK";
const std::string ClientHandler::HTTP_RESPOND_NOT_FOUND = "404 Not Found";
const std::string ClientHandler::HTTP_RESPOND_ENDING_SINGLE = "\r\n";
const std::string ClientHandler::HTTP_RESPOND_ENDING_DOUBLE = "\r\n\r\n";

const std::string ClientHandler::HTTP_FULL_RESPONSE_ERROR = HTTP_STANDARD + HTTP_RESPOND_NOT_FOUND + HTTP_RESPOND_ENDING_DOUBLE;

void ClientHandler::send_response(int client_fd, const std::string &response_string)
{
    send(client_fd, response_string.c_str(), response_string.size(), 0);
}

std::string ClientHandler::create_response_with_body(const std::string &status, const std::string &body)
{
    std::string response = HTTP_STANDARD + status + HTTP_RESPOND_ENDING_SINGLE;
    response += "Content-Type: text/plain" + HTTP_RESPOND_ENDING_SINGLE;
    response += "Content-Length: " + std::to_string(body.size()) + HTTP_RESPOND_ENDING_DOUBLE;
    response += body;
    return response;
}

void ClientHandler::handle_get_request(const request &req, int client_fd)
{
    std::string sendingString;
    std::cout << req.path << "\n";
    if (req.path == "/" || req.path == "/index.html")
    {
        sendingString = create_response_with_body(HTTP_RESPOND_OK, "");
        send_response(client_fd, sendingString);
        return;
    }
    else if (req.path.substr(0, 5).compare("/echo") == 0)
    {
        sendingString = create_response_with_body(HTTP_RESPOND_OK, req.path.substr(6));
        send_response(client_fd, sendingString);
        return;
    }
    else if (req.path.substr(0, 12).compare("/user-agent") == 0)
    {
        std::string user_agent = HeaderConverter::get_header_value(req.buffer, HeaderType::USER_AGENT);

        sendingString = create_response_with_body(HTTP_RESPOND_OK, user_agent);
        send_response(client_fd, sendingString);
        return;
    }
    else if (req.path.substr(0, 6).compare("/files") == 0)
    {
        std::string file_path = "storage/" + req.path.substr(7);
        if (file_path.find("..") != std::string::npos)
        {
            send_response(client_fd, HTTP_FULL_RESPONSE_ERROR);
            std::cerr << "Invalid file path: " << file_path << std::endl;
            return;
        }

        FILE *file = fopen(file_path.c_str(), "rb");
        if (file)
        {
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            sendingString = HTTP_STANDARD + HTTP_RESPOND_OK + HTTP_RESPOND_ENDING_SINGLE;
            sendingString += "Content-Type: application/octet-stream" + HTTP_RESPOND_ENDING_SINGLE;
            sendingString += "Content-Length: " + std::to_string(file_size) + HTTP_RESPOND_ENDING_DOUBLE;

            char *file_buffer = new char[file_size];
            fread(file_buffer, 1, file_size, file);
            sendingString.append(file_buffer, file_size);
            delete[] file_buffer;
            fclose(file);

            send_response(client_fd, sendingString);
        }
        else
        {
            send_response(client_fd, HTTP_FULL_RESPONSE_ERROR);
            std::cerr << "File not found: " << file_path << std::endl;
            return;
        }
    }
    else
    {
        send_response(client_fd, HTTP_FULL_RESPONSE_ERROR);
        return;
    }

    send_response(client_fd, HTTP_FULL_RESPONSE_ERROR);
}

void ClientHandler::handle_post_request(const request &req, int client_fd)
{
    if (req.path.substr(0, 6).compare("/files") == 0)
    {
        std::string file_path = "storage/" + req.path.substr(7);
        std::string body = req.body;

        std::ofstream file(file_path);
        if (file)
        {
            file << body;
            file.close();

            std::string sendingString = create_response_with_body("201 Created", body);
            send_response(client_fd, sendingString);
            return;
        }
        else
        {
            send_response(client_fd, HTTP_FULL_RESPONSE_ERROR);
            std::cerr << "Failed to create file: " << file_path << std::endl;
            return;
        }
    }
    else
    {
        send_response(client_fd, HTTP_FULL_RESPONSE_ERROR);
        return;
    }
}

std::string ClientHandler::get_request_body(const request &req)
{
    std::string body;
    const char *body_start = strstr(req.buffer, "\r\n\r\n");
    if (body_start)
    {
        body_start += 4; // Skip past the "\r\n\r\n"
        body.assign(body_start);
    }
    return body;
}
void ClientHandler::handle_client(int client_fd)
{
    struct timeval timeout;
    timeout.tv_sec = 30; // 30 seconds
    timeout.tv_usec = 0;
    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    while (true)
    {
        char buffer[1024];
        ssize_t bytes = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes > 0)
        {
            request req;
            req.buffer = buffer;
            req.method = HeaderConverter::recognize_header_request(buffer);
            req.path = HeaderConverter::get_header_value(buffer, req.method);
            req.body = get_request_body(req);

            switch (req.method)
            {
            case HeaderType::GET:
                handle_get_request(req, client_fd);
                break;
            case HeaderType::POST:
                handle_post_request(req, client_fd);
                break;
            default:
                std::cerr << "Unsupported HTTP method\n";
                break;
            }
        }
        else if (bytes == 0)
        {
            // Client closed connection
            break;
        }
        else
        {
            // Error or timeout
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                // Timeout occurred
                std::cerr << "recv timed out\n";
                break;
            }

            // Other error
            std::cerr << "recv failed\n";
            break;
        }
    }
    close(client_fd);
}