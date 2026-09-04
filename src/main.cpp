#include <iostream>

#include "HttpServer.h"

int main() {

    try {

        HttpServer server(8080);

        server.run();

    } catch (const std::exception& error) {

        std::cerr
            << error.what()
            << '\n';

        return 1;
    }

    return 0;
}