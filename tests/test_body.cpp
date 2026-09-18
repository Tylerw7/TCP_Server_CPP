#include <catch2/catch_test_macros.hpp>

#include "http/HttpRequest.h"

using namespace http;

TEST_CASE("Parses a urlencoded form body", "[body]") {
    HttpRequest request;
    request.headers["Content-Type"] = "application/x-www-form-urlencoded";
    request.body = "name=Tyler&age=30";

    request.parse_form_body();

    REQUIRE(request.form_params["name"] == "Tyler");
    REQUIRE(request.form_params["age"] == "30");
}

TEST_CASE("A JSON body is not treated as a form", "[body]") {
    HttpRequest request;
    request.headers["Content-Type"] = "application/json";
    request.body = "{\"name\":\"Tyler\"}";

    request.parse_form_body();

    REQUIRE(request.form_params.empty() == true);
}

TEST_CASE("Content-Type with charset suffix still parses as a form", "[body]") {
    HttpRequest request;
    request.headers["Content-Type"] =
        "application/x-www-form-urlencoded; charset=utf-8";
    request.body = "city=Daly+City";

    request.parse_form_body();

    REQUIRE(request.form_params["city"] == "Daly+City");
}

TEST_CASE("A missing Content-Type leaves form_params empty", "[body]") {
    HttpRequest request;
    request.body = "name=Tyler";

    request.parse_form_body();

    REQUIRE(request.form_params.empty() == true);
}