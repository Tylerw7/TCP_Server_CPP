#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "HttpServer.h"



HttpServer::HttpServer(int port) : server_fd(-1), port(port) {
    setup();
};


void HttpServer::setup() {

    server_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (server_fd == -1) {
        throw std::runtime_error(
            "Failed to create socket"
        );
    }

    int opt = 1;

    if (setsockopt(
            server_fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &opt,
            sizeof(opt)
        ) == -1) {

        close(server_fd);

        throw std::runtime_error(
            "Failed to set SO_REUSEADDR"
        );
    }

    sockaddr_in server_address{};

    server_address.sin_family =
        AF_INET;

    server_address.sin_addr.s_addr =
        htonl(INADDR_LOOPBACK);

    server_address.sin_port =
        htons(port);

    if (bind(
            server_fd,
            (sockaddr*)&server_address,
            sizeof(server_address)
        ) == -1) {

        close(server_fd);

        throw std::runtime_error(
            "Failed to bind socket"
        );
    }

    if (listen(server_fd, 10) == -1) {

        close(server_fd);

        throw std::runtime_error(
            "Failed to listen"
        );
    }

    std::cout
        << "Server listening on "
        << "http://localhost:"
        << port
        << '\n';
}


void HttpServer::run() {

    while (true) {

        int client_fd = accept(
            server_fd,
            nullptr,
            nullptr
        );

        if (client_fd == -1) {

            std::cerr
                << "Failed to accept client\n";

            continue;
        }

        std::cout
            << "Client connected\n";

        handle_client(client_fd);
    }
}



void HttpServer::handle_client(int client_fd) {

}