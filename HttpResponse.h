#pragma once

#include <string>
#include <unordered_map>

#include "HttpStatus.h"


class HttpResponse {
    public:
        HttpStatus status;
        
        std::unordered_map<std::string, std::string> headers;
        std::string body;
};