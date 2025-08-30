#pragma once
#include <string>
#include <iostream>
#include "header_type.h"

class HeaderConverter
{
private:
    static std::string trim_http(const std::string &input);

public:
    static std::string get_header_value(const char *buffer, HeaderType header);
    static std::string get_header_value(const char *buffer, const char *header);
    static HeaderType recognize_header_request(const char *buffer);
};
