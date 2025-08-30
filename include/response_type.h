#pragma once

#include <string>
#include <map>

struct ResponseType
{
    int status_code;
    std::map<std::string, std::string> headers;
    std::string body;
};