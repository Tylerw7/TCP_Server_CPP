#pragma once

#include <string>
#include "http/HttpResponse.h"

namespace http {

    class HttpResponseBuilder {
    public:
        std::string build(const HttpResponse& response);

    // private:
    //     std::string status_text(HttpStatus status);    
};

}