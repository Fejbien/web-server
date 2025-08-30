#pragma once
#include <sys/socket.h>
#include <string>
#include <unistd.h>
#include "header_type.h"
#include "request_type.h"
#include "header_converter.h"

class ClientHandler
{
private:
    static const std::string HTTP_STANDARD;
    static const std::string HTTP_RESPOND_OK;
    static const std::string HTTP_RESPOND_NOT_FOUND;
    static const std::string HTTP_RESPOND_ENDING_SINGLE;
    static const std::string HTTP_RESPOND_ENDING_DOUBLE;

    static void handle_get_request(const request &req, int client_fd);

public:
    static void handle_client(int client_fd);
};
