#pragma once
#include <sys/socket.h>
#include <string>
#include <unistd.h>
#include <fstream>
#include <filesystem>

#include "header_type.h"
#include "request_type.h"
#include "header_converter.h"
#include "response_type.h"
#include "response_type_converter.h"

class ClientHandler
{
private:
    static void send_response(int client_fd, const std::string &response_string);

    static void handle_get_request(const request &req, int client_fd);
    static void handle_post_request(const request &req, int client_fd);

    static std::string get_request_body(const request &req);

public:
    static void handle_client(int client_fd);
};
