#include "Router.h"


void Router::add_route(
    const std::string& method,
    const std::string& path,
    std::function<void()> handler
) {
    routes.push_back({
        method,
        path,
        handler
    });
}

void Router::get(
    const std::string& path,
    std::function<void()> handler
) {
    add_route("GET", path, handler);
}


void Router::post(
    const std::string& path,
    std::function<void()> handler
) {
    add_route("POST", path, handler);
}


void Router::put(
    const std::string& path,
    std::function<void()> handler
) {
    add_route("PUT", path, handler);
}


void Router::patch(
    const std::string& path,
    std::function<void()> handler
) {
    add_route("PATCH", path, handler);
}


void Router::delete_route(
    const std::string& path,
    std::function<void()> handler
) {
    add_route("DELETE", path, handler);
}


bool Router::matches(
    const std::string& method,
    const std::string& path
) const {

    for (const auto& route : routes) {
        if (route.method == method && route.path == path) return true;
    }

    return false;
}