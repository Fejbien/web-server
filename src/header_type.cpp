#include "header_type.h"

std::string HeaderTypeConverter::header_type_to_string(HeaderType type)
{
    switch (type)
    {
    case HeaderType::GET:
        return "GET";
    case HeaderType::PUT:
        return "PUT";
    case HeaderType::POST:
        return "POST";
    case HeaderType::DELETE:
        return "DELETE";
    case HeaderType::CONNECTION:
        return "Connection";
    case HeaderType::USER_AGENT:
        return "User-Agent";
    default:
        return "Unknown";
    }
}

HeaderType HeaderTypeConverter::string_to_header_type(const std::string &str)
{
    if (str == "GET")
        return HeaderType::GET;
    else if (str == "PUT")
        return HeaderType::PUT;
    else if (str == "POST")
        return HeaderType::POST;
    else if (str == "DELETE")
        return HeaderType::DELETE;
    else if (str == "Connection")
        return HeaderType::CONNECTION;
    else if (str == "User-Agent")
        return HeaderType::USER_AGENT;
    else
        throw std::invalid_argument("Unknown header type string");
}
