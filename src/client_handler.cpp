#include "client_handler.h"
#include <filesystem>
#include <regex>
#include <cctype>

void ClientHandler::send_response(int client_fd, const std::string &response_string)
{
    send(client_fd, response_string.c_str(), response_string.size(), 0);
}

void ClientHandler::handle_get_request(const request &req, int client_fd)
{
    std::string sendingString;

    // URL-decode the request path so "%20" becomes ' '
    auto url_decode = [](const std::string &src)
    {
        std::string ret;
        ret.reserve(src.size());
        for (size_t i = 0; i < src.size(); ++i)
        {
            char c = src[i];
            if (c == '%')
            {
                if (i + 2 < src.size() && std::isxdigit((unsigned char)src[i + 1]) && std::isxdigit((unsigned char)src[i + 2]))
                {
                    auto hex_to_int = [](char h) -> int
                    {
                        if (h >= '0' && h <= '9')
                            return h - '0';
                        if (h >= 'a' && h <= 'f')
                            return h - 'a' + 10;
                        if (h >= 'A' && h <= 'F')
                            return h - 'A' + 10;
                        return 0;
                    };
                    char hi = src[i + 1];
                    char lo = src[i + 2];
                    char decoded = (char)((hex_to_int(hi) << 4) + hex_to_int(lo));
                    ret.push_back(decoded);
                    i += 2;
                }
                else
                {
                    // malformed, keep as is
                    ret.push_back(c);
                }
            }
            else if (c == '+')
            {
                ret.push_back(' ');
            }
            else
            {
                ret.push_back(c);
            }
        }
        return ret;
    };

    std::string decoded_path = url_decode(req.path);
    if (decoded_path.substr(0, 5).compare("/echo") == 0)
    {
        ResponseType response = {200, {{"Content-Type", "text/plain"}}, decoded_path.substr(6)};
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
    else if (decoded_path.substr(0, 6).compare("/files") == 0)
    {
        std::string file_path = "storage/" + decoded_path.substr(7);
        std::cout << "Requested file: " << file_path << std::endl;
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
    else if (decoded_path.substr(0, 12) == "/getAllFiles")
    {
        std::string files;
        for (const auto &entry : std::filesystem::directory_iterator("storage"))
        {
            files += entry.path().string().substr(8) + "\n";
        }

        std::cout << files << std::endl;

        ResponseType response = {200, {{"Content-Type", "text/plain"}}, files};
        send_response(client_fd, ResponseTypeConverter::toResponse(response));
        return;
    }

    // public doesnt have "/" bcs of the request path always contains it
    std::string path = decoded_path;
    if (decoded_path == "/")
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
        // Parse headers
        std::string boundary = HeaderConverter::get_boundary_value(req.buffer);
        std::string boundary_marker = "--" + boundary;
        std::string boundary_end = boundary_marker + "--";

        bool in_file = false;
        std::ofstream file;
        std::string leftover;
        std::string buffer_acc = req.body;

        auto process_chunk = [&](const std::string &chunk)
        {
            size_t pos = 0;

            while (true)
            {
                if (!in_file)
                {
                    size_t boundary_pos = chunk.find(boundary_marker, pos);
                    if (boundary_pos == std::string::npos)
                        return;

                    // Skip boundary line
                    size_t header_start = boundary_pos + boundary_marker.size() + 2;
                    size_t header_end = chunk.find("\r\n\r\n", header_start);
                    if (header_end == std::string::npos)
                        return;

                    std::string headers = chunk.substr(header_start, header_end - header_start);

                    // Extract filename
                    std::smatch match;
                    std::regex filename_regex("filename=\"([^\"]+)\"");
                    if (std::regex_search(headers, match, filename_regex))
                    {
                        std::string filename = match.str(1);
                        std::string file_path = "storage/" + filename;
                        file.open(file_path, std::ios::binary);
                        in_file = true;
                    }

                    pos = header_end + 4;
                }
                else
                {
                    size_t boundary_pos = chunk.find(boundary_marker, pos);
                    if (boundary_pos != std::string::npos)
                    {
                        file.write(chunk.data() + pos, boundary_pos - pos - 2);
                        file.close();
                        in_file = false;
                        return; // file finished
                    }
                    else
                    {
                        file.write(chunk.data() + pos, chunk.size() - pos);
                        return;
                    }
                }
            }
        };

        // process already read body
        if (!buffer_acc.empty())
            process_chunk(buffer_acc);

        // keep streaming
        char buf[8192];
        ssize_t bytes_read;
        while ((bytes_read = recv(client_fd, buf, sizeof(buf), 0)) > 0)
        {
            std::string chunk(buf, bytes_read);
            process_chunk(chunk);

            if (!in_file)
                break;
        }

        // Respond
        ResponseType response = {201, {{"Content-Type", "text/plain"}}, "File uploaded!"};
        send_response(client_fd, ResponseTypeConverter::toResponse(response));
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
            std::cout << "Received request: " << buffer << std::endl;
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