#include <iostream>

#include "http/HttpServer.h"
#include "http/Router.h"

using namespace http;

int main() {
    try {
        Router router;

        router.get("/", [](const HttpRequest& request) {
            HttpResponse response;
            response.status = HttpStatus::OK;
            response.headers["Content-Type"] = "text/plain";
            response.body = "Hello from the framework";
            return response;
        });

        router.get("/search", [](const HttpRequest& request) {
            HttpResponse response;
            response.status = HttpStatus::OK;
            response.headers["Content-Type"] = "text/plain";

            auto it = request.query_params.find("q");
            std::string term = (it != request.query_params.end()) ? it->second : "(none)";

            response.body = "You searched for: " + term;
            return response;
        });

        router.get("/", [](const HttpRequest& request) {

            HttpResponse response;

            response.status = HttpStatus::OK;

            response.headers["Content-Type"] =
                "text/plain";

            response.body =
                "Method: " + request.method +
                "\nPath: " + request.path +
                "\nVersion: " + request.version;

            return response;
        });

        HttpServer server(8080, router);
        server.run();

    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}