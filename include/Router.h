#pragma once

#include <string>
#include <vector>
#include <functional>

#include "Route.h"


class Router {

    public:
        void add_route(
            const std::string& method,
            const std::string& path,
            std::function<void()> handler
        );

        void get(
            const std::string& path,
            std::function<void()> handler
        );

        void post(
            const std::string& path,
            std::function<void()> handler
        );

        void put(
            const std::string& path,
            std::function<void()> handler
        );

        void patch(
            const std::string& path,
            std::function<void()> handler
        );

        void delete_route(
            const std::string& path,
            std::function<void()> handler
        );

        bool matches(
            const std::string& method,
            const std::string& path
        ) const;

    private:
        std::vector<Route> routes;    
};