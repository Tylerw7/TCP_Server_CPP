#pragma once

#include <string>
#include <vector>
#include <functional>

#include "http/Route.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"

namespace http {

    class Router {

public:

    void add_route(
        const std::string& method,
        const std::string& path,
        std::function<HttpResponse(const HttpRequest&)> handler
    );

    void get(
        const std::string& path,
        std::function<HttpResponse(const HttpRequest&)> handler
    );

    void post(
        const std::string& path,
        std::function<HttpResponse(const HttpRequest&)> handler
    );

    void put(
        const std::string& path,
        std::function<HttpResponse(const HttpRequest&)> handler
    );

    void patch(
        const std::string& path,
        std::function<HttpResponse(const HttpRequest&)> handler
    );

    void delete_route(
        const std::string& path,
        std::function<HttpResponse(const HttpRequest&)> handler
    );

    HttpResponse handle(
        const HttpRequest& request
    );

private:

    std::vector<Route> routes;
};


}