#pragma once
#include <string>
#include <map>
#include "header_type.h"

struct request
{
    HeaderType method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;
    char *buffer;
};
