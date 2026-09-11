#pragma once

#include <string>
#include <functional>

#include "http/HttpResponse.h"
#include "http/HttpRequest.h"

namespace http {

    struct Route {
    std::string method;
    std::string path;

    std::function<HttpResponse(const HttpRequest&)> handler;
};

}