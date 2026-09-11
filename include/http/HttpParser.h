#pragma once

#include <string>
#include "http/HttpRequest.h"

namespace http {


    class HttpParser {

    public:
        
        bool parse(const std::string& raw_request, HttpRequest& request);
};
}

