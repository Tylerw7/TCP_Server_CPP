#include <iostream>

#include "http/HttpServer.h"
#include "http/Router.h"

#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>



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


        router.get("/api/status", [](const HttpRequest& request) {
            nlohmann::json data;
            data["service"] = "my-framework";
            data["status"] = "ok";
            data["version"] = "1.0";

            return HttpResponse::json(HttpStatus::OK, data);
        });


        router.get("/api/users/:id", [](const HttpRequest& request) {
            std::string id = request.path_params.at("id");

            if (id != "42") {
                return HttpResponse::error(HttpStatus::NotFound, "user not found");
            }

            nlohmann::json user;
            user["id"] = id;
            user["name"] = "Tyler";

            return HttpResponse::json(HttpStatus::OK, user);
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


        router.get("/users/:id", [](const HttpRequest& request) {
            HttpResponse response;
            response.status = HttpStatus::OK;
            response.headers["Content-Type"] = "text/plain";

            std::string id = request.path_params.at("id");
            response.body = "Requested user: " + id;

            return response;
        });


        router.post("/submit", [](const HttpRequest& request) {
            HttpResponse response;
            response.status = HttpStatus::OK;
            response.headers["Content-Type"] = "text/plain";

            auto it = request.form_params.find("name");
            std::string name = (it != request.form_params.end()) ? it->second : "(none)";

            response.body = "Form submitted by: " + name;
            return response;
        });


        router.get("/slow", [](const HttpRequest& request) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            HttpResponse response;
            response.status = HttpStatus::OK;
            response.body = "done";
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