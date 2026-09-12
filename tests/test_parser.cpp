#include <catch2/catch_test_macros.hpp>

#include "http/HttpParser.h"
#include "http/HttpRequest.h"

using namespace http;



TEST_CASE("Parses a simple GET request", "[parser]") {
    HttpParser parser;
    HttpRequest request;

    std::string raw =
        "GET /hello HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    bool ok = parser.parse(raw, request);

    REQUIRE(ok == true);
    REQUIRE(request.method == "GET");
    REQUIRE(request.path == "/hello");
    REQUIRE(request.version == "HTTP/1.1");
    REQUIRE(request.headers["Host"] == "localhost");
}

TEST_CASE("Rejects an invalid method", "[parser]") {
    HttpParser parser;
    HttpRequest request;

    std::string raw = "FETCH / HTTP/1.1\r\n\r\n";

    REQUIRE(parser.parse(raw, request) == false);
}

TEST_CASE("Extracts the body", "[parser]") {
    HttpParser parser;
    HttpRequest request;

    std::string raw =
        "POST /echo HTTP/1.1\r\n"
        "Content-Length: 2\r\n"
        "\r\n"
        "{}";

    REQUIRE(parser.parse(raw, request) == true);
    REQUIRE(request.body == "{}");
}
