#pragma once

#include <string>
#include "HttpRequest.h"

class HttpParser {

    public:
        
        bool parse(const std::string& raw_request, HttpRequest& request);
};