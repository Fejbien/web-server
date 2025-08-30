#pragma once
#include <string>

#include "response_type.h"

class ResponseTypeConverter
{
private:
    static std::string statusCodeString(int code);

public:
    static std::string toResponse(ResponseType response);
};