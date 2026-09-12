#include <catch2/catch_test_macros.hpp>
#include "http/Router.h"

using namespace http;

TEST_CASE("Middleware runs outermost-first and wraps the route", "[middleware]") {
    Router router;
    std::string trace;

    router.use([&](const http::HttpRequest& req,
                   std::function<http::HttpResponse(const http::HttpRequest&)> next) {
        trace += "A-in ";
        auto res = next(req);
        trace += "A-out ";
        return res;
    });
}    