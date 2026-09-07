#include "Router.h"
#include "HttpResponse.h"
#include "HttpStatus.h"

#include <iostream>


void Router::add_route(
    const std::string& method,
    const std::string& path,
    std::function<HttpResponse(const HttpRequest&)> handler
) {
    routes.push_back({
        method,
        path,
        handler
    });
}


void Router::get(
    const std::string& path,
    std::function<HttpResponse(const HttpRequest&)> handler
) {
    add_route("GET", path, handler);
}


void Router::post(
    const std::string& path,
    std::function<HttpResponse(const HttpRequest&)> handler
) {
    add_route("POST", path, handler);
}


void Router::put(
    const std::string& path,
    std::function<HttpResponse(const HttpRequest&)> handler
) {
    add_route("PUT", path, handler);
}


void Router::patch(
    const std::string& path,
    std::function<HttpResponse(const HttpRequest&)> handler
) {
    add_route("PATCH", path, handler);
}


void Router::delete_route(
    const std::string& path,
    std::function<HttpResponse(const HttpRequest&)> handler
) {
    add_route("DELETE", path, handler);
}


HttpResponse Router::handle(
    const HttpRequest& request
) {
    for (const auto& route : routes) {

        if (route.method == request.method &&
            route.path == request.path) {

            return route.handler(request);
        }
    }

    HttpResponse response;

    response.status = HttpStatus::NotFound;
    response.headers["Content-Type"] = "text/plain";
    response.body = "404 - NotFound";

    return response;
}