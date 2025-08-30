#include "response_type_converter.h"

std::string ResponseTypeConverter::statusCodeString(int code)
{
    switch (code)
    {
    case 200:
        return "200 OK";
    case 201:
        return "201 Created";
    case 204:
        return "204 No Content";
    case 404:
        return "404 Not Found";
    case 500:
        return "500 Internal Server Error";
    default:
        return std::to_string(code) + " Unknown to server";
    }
}

std::string ResponseTypeConverter::toResponse(ResponseType response)
{
    std::string returnString;

    returnString += "HTTP/1.1 " + statusCodeString(response.status_code) + " \r\n";
    for (const auto &header : response.headers)
    {
        returnString += header.first + ": " + header.second + "\r\n";
    }

    if (response.headers.find("Content-Length") == response.headers.end())
    {
        returnString += "Content-Length: " + std::to_string(response.body.size()) + "\r\n";
    }
    returnString += "\r\n";
    returnString += response.body;

    return returnString;
}