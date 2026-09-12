#include "http/HttpRequest.h"
#include "http/HttpParser.h"
#include <catch2/catch_test_macros.hpp>


using namespace http;



TEST_CASE("Parses query parameters and cleans the path", "[query]") {
    http::HttpParser parser;
    http::HttpRequest request;

    std::string raw =
        "GET /search?q=cats&limit=10 HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    REQUIRE(parser.parse(raw, request) == true);
    REQUIRE(request.path == "/search");
    REQUIRE(request.query_params["q"] == "cats");
    REQUIRE(request.query_params["limit"] == "10");
}

TEST_CASE("A path with no query string still works", "[query]") {
    http::HttpParser parser;
    http::HttpRequest request;

    std::string raw = "GET /plain HTTP/1.1\r\n\r\n";

    REQUIRE(parser.parse(raw, request) == true);
    REQUIRE(request.path == "/plain");
    REQUIRE(request.query_params.empty() == true);
}