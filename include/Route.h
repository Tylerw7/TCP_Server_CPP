#pragma once

#include <string>
#include <functional>

#include "HttpResponse.h"

struct Route {
    std::string method;
    std::string path;

    std::function<HttpResponse()> handler;
};