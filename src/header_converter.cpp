#include "header_converter.h"

std::string HeaderConverter::trim_http(const std::string &input)
{
    std::string result = input;
    size_t pos = result.find("HTTP");
    if (pos != std::string::npos)
    {
        result = result.substr(0, pos); // Keeps everything before "HTTP"
        result.erase(result.find_last_not_of(" \t") + 1);
    }
    return result;
}

std::string HeaderConverter::get_header_value(const char *buffer, HeaderType header)
{
    switch (header)
    {
    case HeaderType::GET:
    {
        std::string input = get_header_value(buffer, "GET ");
        return trim_http(input);
    }
    case HeaderType::PUT:
    {
        std::string input = get_header_value(buffer, "PUT ");
        return trim_http(input);
    }
    case HeaderType::POST:
    {
        std::string input = get_header_value(buffer, "POST ");
        return trim_http(input);
    }
    case HeaderType::DELETE:
    {
        std::string input = get_header_value(buffer, "DELETE ");
        return trim_http(input);
    }
    case HeaderType::CONNECTION:
        return get_header_value(buffer, "Connection: ");
    case HeaderType::USER_AGENT:
        return get_header_value(buffer, "User-Agent: ");
    default:
        return "";
    }
}

std::string HeaderConverter::get_header_value(const char *buffer, const char *header)
{
    std::string header_value;
    const char *header_start = strstr(buffer, header);
    if (header_start)
    {
        header_start += strlen(header);
        const char *header_end = strstr(header_start, "\r\n");
        if (header_end)
        {
            header_value.assign(header_start, header_end);
        }
    }
    return header_value;
}

HeaderType HeaderConverter::recognize_header_request(const char *buffer)
{
    // Trims till first space
    std::string header(buffer);
    header = header.substr(0, header.find(" "));
    if (header == "GET")
    {
        std::cout << "Recognized GET request\n";
        return HeaderType::GET;
    }
    else if (header == "PUT")
    {
        std::cout << "Recognized PUT request\n";
        return HeaderType::PUT;
    }
    else if (header == "POST")
    {
        std::cout << "Recognized POST request\n";
        return HeaderType::POST;
    }
    else if (header == "DELETE")
    {
        std::cout << "Recognized DELETE request\n";
        return HeaderType::DELETE;
    }
    else
        throw std::invalid_argument("Unknown header type");
}
