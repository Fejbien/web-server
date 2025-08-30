#pragma once
#include <string>

enum class HeaderType
{
    GET,
    PUT,
    POST,
    DELETE,
    CONNECTION,
    USER_AGENT,
};

class HeaderTypeConverter
{
public:
    static std::string header_type_to_string(HeaderType type);
    static HeaderType string_to_header_type(const std::string &str);
};