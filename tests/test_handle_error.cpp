#include <catch2/catch_test_macros.hpp>
#include "http/Router.h"

using namespace http;

TEST_CASE("A throwing handler becomes a 500", "[errors]") {
    Router router;

    router.get("/boom", [](const HttpRequest&) -> HttpResponse {
        throw std::runtime_error("handler blew up");
    });

    HttpRequest request;
    request.method = "GET";
    request.path = "/boom";

    HttpResponse response = router.handle(request);

    REQUIRE(response.status == HttpStatus::InternalServerError);
}