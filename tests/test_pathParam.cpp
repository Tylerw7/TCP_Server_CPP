#include <catch2/catch_test_macros.hpp>

#include "http/HttpStatus.h"
#include "http/HttpRequest.h"
#include "http/Router.h"

using namespace http;


TEST_CASE("Path parameter is captured", "[params]") {
    Router router;

    router.get("/users/:id", [](const HttpRequest& request) {
        HttpResponse response;
        response.status = HttpStatus::OK;
        response.body = request.path_params.at("id");
        return response;
    });

    HttpRequest request;
    request.method = "GET";
    request.path = "/users/42";

    HttpResponse response = router.handle(request);

    REQUIRE(response.status == HttpStatus::OK);
    REQUIRE(response.body == "42");
}

TEST_CASE("A different segment count does not match", "[params]") {
    Router router;

    router.get("/users/:id", [](const HttpRequest&) {
        HttpResponse r;
        r.status = HttpStatus::OK;
        return r;
    });

    HttpRequest request;
    request.method = "GET";
    request.path = "/users/42/extra";

    HttpResponse response = router.handle(request);

    REQUIRE(response.status == HttpStatus::NotFound);
}