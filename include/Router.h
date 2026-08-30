#pragma once

#include <string>
#include <vector>

#include "Route.h"


class Router {

    public:
        void add_route(
            const std::string& method,
            const std::string& path
        );

        bool matches(
            const std::string& method,
            const std::string& path
        ) const;

    private:
        std::vector<Route> routes;    
};