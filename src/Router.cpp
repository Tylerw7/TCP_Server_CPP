#include "Router.h"


void Router::add_route(
    const std::string& method,
    const std::string& path
) {
    routes.push_back({
        method,
        path
    });
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