#pragma once

#include <string>
#include <functional>

struct Route {
    std::string method;
    std::string path;

    std::function<void()> handler;
};