#pragma once

#include <string>
#include <vector>
#include <functional>

#include "Route.h"
#include "HttpResponse.h"

class Router {

public:

    void add_route(
        const std::string& method,
        const std::string& path,
        std::function<HttpResponse()> handler
    );

    void get(
        const std::string& path,
        std::function<HttpResponse()> handler
    );

    void post(
        const std::string& path,
        std::function<HttpResponse()> handler
    );

    void put(
        const std::string& path,
        std::function<HttpResponse()> handler
    );

    void patch(
        const std::string& path,
        std::function<HttpResponse()> handler
    );

    void delete_route(
        const std::string& path,
        std::function<HttpResponse()> handler
    );

    HttpResponse handle(
        const std::string& method,
        const std::string& path
    );

private:

    std::vector<Route> routes;
};