#include <iostream>

#include "http/HttpsServer.h"
#include "http/Router.h"

using namespace http;

int main() {
    try {
        Router router;

        router.get("/", [](const HttpRequest& request) {
            HttpResponse response;
            response.status = HttpStatus::OK;
            response.headers["Content-Type"] = "text/plain";
            response.body = "Hello over HTTPS!";
            return response;
        });

        HttpsServer server(8443, router, "cert.pem", "key.pem");
        server.run();

    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}