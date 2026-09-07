#pragma once

#include <string>
#include <functional>

#include "HttpResponse.h"
#include "HttpRequest.h"

struct Route {
    std::string method;
    std::string path;

    std::function<HttpResponse(const HttpRequest&)> handler;
};