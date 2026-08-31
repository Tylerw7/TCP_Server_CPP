#pragma once

#include <string>
#include "HttpResponse.h"

class HttpResponseBuilder {
    public:
        std::string build(const HttpResponse& response);

    // private:
    //     std::string status_text(HttpStatus status);    
};