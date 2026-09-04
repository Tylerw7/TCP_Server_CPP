
#include <stdexcept>
#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <algorithm>

#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "HttpRequest.h"
#include "HttpParser.h"
#include "HttpResponseBuilder.h"
#include "Router.h"
#include "HttpServer.h"



// Global Variables
constexpr size_t MAX_BODY_SIZE = 1024 * 1024;



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



bool send_all(
    int socket_fd,
    const char* data,
    size_t length
) {

    size_t total_sent = 0;

    while (total_sent < length) {

        ssize_t bytes_sent = send(
            socket_fd,
            data + total_sent,
            length - total_sent,
            0
        );

        if (bytes_sent <= 0) {
            return false;
        }

        total_sent += bytes_sent;
    }

    return true;
}


bool parse_content_length(
    const HttpRequest& request,
    size_t& content_length
) {

    auto header =
        request.headers.find("Content-Length");

    if (header == request.headers.end()) {

        content_length = 0;

        return true;
    }

    try {

        size_t position = 0;

        unsigned long long value =
            std::stoull(
                header->second,
                &position
            );

        if (position != header->second.size()) {
            return false;
        }

        content_length =
            static_cast<size_t>(value);

    } catch (...) {

        return false;
    }

    return true;
}





void HttpServer::handle_client(int client_fd) {

    HttpParser parser;

    HttpResponseBuilder response_builder;

    Router router;

    router.get("/", []() {

        HttpResponse response;

        response.status = HttpStatus::OK;

        response.headers["Content-Type"] =
            "text/plain";

        response.body =
            "Welcome to Tylers server";

        return response;
    });


    char buffer[4096];

    std::string raw_request;


    while (
        raw_request.find("\r\n\r\n")
        == std::string::npos
    ) {

        ssize_t bytes_received = recv(
            client_fd,
            buffer,
            sizeof(buffer),
            0
        );

        if (bytes_received == 0) {

            std::cout
                << "Client closed connection\n";

            close(client_fd);

            return;
        }

        if (bytes_received == -1) {

            std::cerr
                << "recv() failed: "
                << strerror(errno)
                << '\n';

            close(client_fd);

            return;
        }

        raw_request.append(
            buffer,
            bytes_received
        );
    }


    HttpRequest request;

    bool parsed =
        parser.parse(
            raw_request,
            request
        );


    if (!parsed) {

        std::cerr
            << "Failed to parse HTTP request\n";

        close(client_fd);

        return;
    }


    size_t content_length = 0;

    if (!parse_content_length(
            request,
            content_length
        )) {

        std::cerr
            << "Invalid Content-Length\n";

        close(client_fd);

        return;
    }


    if (content_length > MAX_BODY_SIZE) {

        std::cerr
            << "Request body too large\n";

        close(client_fd);

        return;
    }


    size_t body_bytes_received =
        request.body.size();


    while (
        body_bytes_received
        < content_length
    ) {

        size_t remaining =
            content_length
            - body_bytes_received;


        size_t bytes_to_receive =
            std::min(
                remaining,
                sizeof(buffer)
            );


        ssize_t bytes_received = recv(
            client_fd,
            buffer,
            bytes_to_receive,
            0
        );


        if (bytes_received == 0) {

            std::cerr
                << "Client closed connection "
                << "before body was complete\n";

            close(client_fd);

            return;
        }


        if (bytes_received == -1) {

            std::cerr
                << "recv() failed: "
                << strerror(errno)
                << '\n';

            close(client_fd);

            return;
        }


        request.body.append(
            buffer,
            bytes_received
        );


        body_bytes_received +=
            bytes_received;
    }


    std::cout
        << "\nMethod: "
        << request.method
        << '\n';

    std::cout
        << "Path: "
        << request.path
        << '\n';

    std::cout
        << "Version: "
        << request.version
        << '\n';


    std::cout
        << "\nHeaders:\n";


    for (const auto& header :
         request.headers) {

        std::cout
            << header.first
            << " = "
            << header.second
            << '\n';
    }


    std::cout
        << "\nBody:\n"
        << request.body
        << '\n';


    HttpResponse response;


    if (
        request.method == "GET"
        && request.path == "/"
    ) {

        response.status =
            HttpStatus::OK;

        response.headers["Content-Type"] =
            "text/plain";

        response.body =
            "Welcome to Tylers server";

    } else {

        response.status =
            HttpStatus::NotFound;

        response.headers["Content-Type"] =
            "text/plain";

        response.body =
            "404 - Not Found";
    }


    std::string response_data =
        response_builder.build(
            response
        );


    send_all(
        client_fd,
        response_data.c_str(),
        response_data.size()
    );


    close(client_fd);

    std::cout
        << "Client disconnected\n";
}