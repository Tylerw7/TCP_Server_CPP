#include <catch2/catch_test_macros.hpp>

#include "http/HttpResponse.h"

using namespace http;

TEST_CASE("json() builds a response with the right status and body", "[json]") {
    nlohmann::json data;
    data["name"] = "Tyler";
    data["age"] = 30;

    HttpResponse response = HttpResponse::json(HttpStatus::OK, data);

    REQUIRE(response.status == HttpStatus::OK);
    REQUIRE(response.headers["Content-Type"] == "application/json");

    // Parse the body back and confirm it round-trips.
    nlohmann::json parsed = nlohmann::json::parse(response.body);
    REQUIRE(parsed["name"] == "Tyler");
    REQUIRE(parsed["age"] == 30);
}

TEST_CASE("error() builds a standard error shape", "[json]") {
    HttpResponse response =
        HttpResponse::error(HttpStatus::NotFound, "user not found");

    REQUIRE(response.status == HttpStatus::NotFound);
    REQUIRE(response.headers["Content-Type"] == "application/json");

    nlohmann::json parsed = nlohmann::json::parse(response.body);
    REQUIRE(parsed["error"] == "user not found");
}