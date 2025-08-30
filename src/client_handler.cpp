#include "client_handler.h"
#include <filesystem>

void ClientHandler::send_response(int client_fd, const std::string &response_string)
{
    send(client_fd, response_string.c_str(), response_string.size(), 0);
}

void ClientHandler::handle_get_request(const request &req, int client_fd)
{
    std::string sendingString;
    // // std::cout << req.path << "\n";
    // if (req.path == "/" || req.path == "/index.html")
    // {
    //     ResponseType response = {200, {{"Content-Type", "text/html"}}, ""};
    //     send_response(client_fd, ResponseTypeConverter::toResponse(response));
    //     return;
    // }
    if (req.path.substr(0, 5).compare("/echo") == 0)
    {
        ResponseType response = {200, {{"Content-Type", "text/plain"}}, req.path.substr(6)};
        send_response(client_fd, ResponseTypeConverter::toResponse(response));
        return;
    }
    else if (req.path.substr(0, 12).compare("/user-agent") == 0)
    {
        std::string user_agent = HeaderConverter::get_header_value(req.buffer, HeaderType::USER_AGENT);

        ResponseType response = {200, {{"Content-Type", "text/plain"}}, user_agent};
        send_response(client_fd, ResponseTypeConverter::toResponse(response));
        return;
    }
    else if (req.path.substr(0, 6).compare("/files") == 0)
    {
        std::string file_path = "storage/" + req.path.substr(7);
        if (file_path.find("..") != std::string::npos)
        {
            ResponseType response = {400, {{"Content-Type", "text/plain"}}, "Invalid file path"};
            send_response(client_fd, ResponseTypeConverter::toResponse(response));
            std::cerr << "Invalid file path: " << file_path << std::endl;
            return;
        }

        FILE *file = fopen(file_path.c_str(), "rb");
        if (file)
        {
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            ResponseType response = {200, {{"Content-Type", "application/octet-stream"}}, ""};
            response.headers["Content-Length"] = std::to_string(file_size);

            char *file_buffer = new char[file_size];
            fread(file_buffer, 1, file_size, file);
            response.body.append(file_buffer, file_size);
            delete[] file_buffer;

            send_response(client_fd, ResponseTypeConverter::toResponse(response));
            fclose(file);
        }
        else
        {
            ResponseType response = {404, {{"Content-Type", "text/plain"}}, "File not found"};
            send_response(client_fd, ResponseTypeConverter::toResponse(response));
            std::cerr << "File not found: " << file_path << std::endl;
            return;
        }
    }

    // public doesnt have "/" bcs of the request path always contains it
    std::string path = req.path;
    if (req.path == "/")
        path = "/index.html";

    std::string file_path = "public" + path;
    std::ifstream file(file_path);
    if (file.good())
    {
        std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        std::string content_type = "text/plain";
        if (file_path.ends_with(".html"))
            content_type = "text/html";
        else if (file_path.ends_with(".css"))
            content_type = "text/css";
        else if (file_path.ends_with(".js"))
            content_type = "application/javascript";

        ResponseType response = {200, {{"Content-Type", content_type}}, body};
        send_response(client_fd, ResponseTypeConverter::toResponse(response));
        return;
    }

    ResponseType response = {400, {{"Content-Type", "text/plain"}}, "Bad Request"};
    send_response(client_fd, ResponseTypeConverter::toResponse(response));
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

            ResponseType response = {201, {{"Content-Type", "text/plain"}}, body};
            std::string response_string = ResponseTypeConverter::toResponse(response);
            send_response(client_fd, response_string);
            return;
        }
        else
        {
            ResponseType response = {500, {{"Content-Type", "text/plain"}}, "Failed to create file"};
            send_response(client_fd, ResponseTypeConverter::toResponse(response));
            std::cerr << "Failed to create file: " << file_path << std::endl;
            return;
        }
    }
    else
    {
        ResponseType response = {404, {{"Content-Type", "text/plain"}}, "Not Found"};
        send_response(client_fd, ResponseTypeConverter::toResponse(response));
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

            // if (HeaderConverter::get_header_value(buffer, "Connection: ").compare("close") == 0)
            // {
            //     close(client_fd);
            //     return;
            // }

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