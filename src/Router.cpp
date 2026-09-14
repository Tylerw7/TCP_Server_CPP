#include "http/Router.h"
#include "http/HttpResponse.h"
#include "http/HttpStatus.h"

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>


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

// Segment Matcher
static std::vector<std::string> split_path(const std::string& path) {
    std::vector<std::string> segments;
    size_t start = 0;

    while (start < path.size()) {
        if (path[start] == '/') {
            start++;
            continue;
        }
        size_t slash = path.find('/', start);
        if (slash == std::string::npos) {
            slash = path.size();
        }
        segments.push_back(path.substr(start, slash - start));
        start = slash;
    }

    return segments;
}


// Matcher
static bool match_route(
    const std::string& pattern,
    const std::string& path,
    std::unordered_map<std::string, std::string>& params
) {
    std::vector<std::string> pattern_parts = split_path(pattern);
    std::vector<std::string> path_parts = split_path(path);

    if (pattern_parts.size() != path_parts.size()) {
        return false;
    }

    std::unordered_map<std::string, std::string> captured;

    for (size_t i = 0; i < pattern_parts.size(); i++) {
        const std::string& p = pattern_parts[i];

        if (!p.empty() && p[0] == ':') {
            // Parameter segment - capture the value
            captured[p.substr(1)] = path_parts[i];
        } else if (p != path_parts[i]) {
            return false;
        }
    }

    params = captured;
    return true;
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
                if (route.method != req.method) continue;

                std::unordered_map<std::string, std::string> params;


                if (match_route(route.path, req.path, params)) {
                    HttpRequest matched = req;          // mutable copy
                    matched.path_params = params;       // attach captured params
                    return route.handler(matched);
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
