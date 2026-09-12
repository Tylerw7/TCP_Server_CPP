#include "http/Router.h"
#include "http/HttpResponse.h"
#include "http/HttpStatus.h"

#include <iostream>


using namespace http;



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
    std::function<HttpResponse(const HttpRequest&)> next =
        [this](const HttpRequest& req) {
            for (const auto& route : routes) {
                if (route.method == req.method &&
                    route.path == req.path) {
                    return route.handler(req);
                }
            }

            HttpResponse response;
            response.status = HttpStatus::NotFound;
            response.headers["Content-Type"] = "text/plain";
            response.body = "404 - NotFound";
            return response;
        };


    for (auto it = middlewares.rbegin(); it != middlewares.rend(); ++it) {
        Middleware current = *it;
        auto previous_next = next;
        next = [current, previous_next](const HttpRequest& req) {
            return current(req, previous_next);
        };
    }

    // ... Next lambda
    auto make_500 = []() {
        HttpResponse response;
        response.status = HttpStatus::InternalServerError;
        response.headers["Content-Type"] = "text/plain";
        response.body = "500 - Internal Server Error";
        return response;
    };

    
    try {
        return next(request);
    } catch (const std::exception& error) {
        return make_500();
    } catch (...) {
        return make_500();
    }
}


// Middleware "Use"

void Router::use(Middleware middleware) {
    middlewares.push_back(middleware);
}
