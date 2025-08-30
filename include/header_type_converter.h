#pragma once
#include <string>
#include "header_type.h"

class HeaderTypeConverter
{
public:
    static std::string header_type_to_string(HeaderType type);
    static HeaderType string_to_header_type(const std::string &str);
};