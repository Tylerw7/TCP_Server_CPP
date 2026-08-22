#pragma once

#include <string>
#include "HttpRequest.h"

class HttpParser {

    public:
        
        HttpRequest parse(const std::string& raw_request);
};