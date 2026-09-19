#include <catch2/catch_test_macros.hpp>
#include "http/HttpRequest.h"

// Declared in HttpServer.cpp, inside namespace http.
namespace http {
    bool should_keep_alive(const HttpRequest& request);
}

using namespace http;

TEST_CASE("HTTP/1.1 defaults to keep-alive", "[keepalive]") {
    HttpRequest request;
    request.version = "HTTP/1.1";
    REQUIRE(should_keep_alive(request) == true);
}

TEST_CASE("Connection: close forces close", "[keepalive]") {
    HttpRequest request;
    request.version = "HTTP/1.1";
    request.headers["Connection"] = "close";
    REQUIRE(should_keep_alive(request) == false);
}

TEST_CASE("Connection: keep-alive is honored", "[keepalive]") {
    HttpRequest request;
    request.version = "HTTP/1.1";
    request.headers["Connection"] = "keep-alive";
    REQUIRE(should_keep_alive(request) == true);
}