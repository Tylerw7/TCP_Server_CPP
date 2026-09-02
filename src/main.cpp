#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <csignal>
#include <algorithm>

#include <sys/socket.h>
#include <netinet/in.h>

#include "HttpRequest.h"
#include "HttpParser.h"
#include "HttpResponseBuilder.h"
#include "Router.h"

// Global Variables
volatile sig_atomic_t running = true;
constexpr size_t MAX_BODY_SIZE = 1024 * 1024;


// Signal Handler
void handle_signal(int signal) {
    running = false;
}


// Function Declarations
bool send_all(
    int socket_fd,
    const char* data,
    size_t length
);

bool parse_content_length(const HttpRequest request, size_t& content_length);




int main() {

    std::signal(SIGINT, handle_signal);


    // ---------------------------------------------------------
    // CREATE SOCKET
    // ---------------------------------------------------------

    // AF_INET:
    // Use IPv4.
    //
    // SOCK_STREAM:
    // Use a TCP stream.
    //
    // 0:
    // Use the default protocol for this socket type.

    int server_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (server_fd == -1) {

        std::cerr
            << "Failed to create socket\n";

        return 1;
    }


    // ---------------------------------------------------------
    // SO_REUSEADDR
    // ---------------------------------------------------------

    int opt = 1;

    int result = setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

    if (result == -1) {

        std::cerr
            << "Failed to set SO_REUSEADDR\n";

        close(server_fd);

        return 1;
    }


    // ---------------------------------------------------------
    // SERVER ADDRESS
    // ---------------------------------------------------------

    sockaddr_in server_address{};

    // IPv4
    server_address.sin_family = AF_INET;

    // 127.0.0.1
    server_address.sin_addr.s_addr =
        htonl(INADDR_LOOPBACK);

    // Port 8080
    server_address.sin_port =
        htons(8080);


    // ---------------------------------------------------------
    // BIND
    // ---------------------------------------------------------

    result = bind(
        server_fd,
        (sockaddr*)&server_address,
        sizeof(server_address)
    );

    if (result == -1) {

        std::cerr
            << "Failed to bind socket\n";

        close(server_fd);

        return 1;
    }


    // ---------------------------------------------------------
    // LISTEN
    // ---------------------------------------------------------

    result = listen(
        server_fd,
        10
    );

    if (result == -1) {

        std::cerr
            << "Failed to listen on socket\n";

        close(server_fd);

        return 1;
    }


    std::cout
        << "Server listening on "
        << "http://localhost:8080\n";


    // ---------------------------------------------------------
    // HTTP PARSER & RESPONSE BUILDER
    // ---------------------------------------------------------

    HttpParser parser;

    HttpResponseBuilder response_builder;


    // ---------------------------------------------------------
    // ROUTER
    // ---------------------------------------------------------

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


    // ---------------------------------------------------------
    // ACCEPT CLIENTS
    // ---------------------------------------------------------

    while (running) {

        int client_fd = accept(
            server_fd,
            nullptr,
            nullptr
        );


        if (client_fd == -1) {

            if (!running) {
                break;
            }

            std::cerr
                << "Failed to accept client\n";

            continue;
        }


        std::cout
            << "Client connected!\n";

        std::cout
            << "Client socket: "
            << client_fd
            << "\n";


        // -----------------------------------------------------
        // RECEIVE HTTP REQUEST
        // -----------------------------------------------------

        char buffer[4096];

        std::string raw_request;


        // -----------------------------------------------------
        // RECEIVE HEADERS
        // -----------------------------------------------------

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

                break;
            }


            if (bytes_received == -1) {

                std::cerr
                    << "recv() failed: "
                    << strerror(errno)
                    << '\n';

                break;
            }


            raw_request.append(
                buffer,
                bytes_received
            );
        }


        // -----------------------------------------------------
        // PARSE HTTP REQUEST
        // -----------------------------------------------------

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

            continue;
        }


        // -----------------------------------------------------
        // GET CONTENT-LENGTH
        // -----------------------------------------------------

        size_t content_length = 0;

        if (!parse_content_length(
                request,
                content_length
            )) {

            std::cerr
                << "Invalid Content-Length\n";

            close(client_fd);

            continue;
        }

        if (content_length > MAX_BODY_SIZE) {

            std::cerr
                << "Request body too large\n";

            close(client_fd);

            continue;
        }


        // -----------------------------------------------------
        // RECEIVE REMAINING BODY
        // -----------------------------------------------------

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

                break;
            }


            if (bytes_received == -1) {

                std::cerr
                    << "recv() failed: "
                    << strerror(errno)
                    << '\n';

                break;
            }


            request.body.append(
                buffer,
                bytes_received
            );


            body_bytes_received +=
                bytes_received;
        }


        // -----------------------------------------------------
        // DISPLAY REQUEST
        // -----------------------------------------------------

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


        // -----------------------------------------------------
        // TEMPORARY RESPONSE
        // -----------------------------------------------------

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


        // -----------------------------------------------------
        // BUILD RESPONSE
        // -----------------------------------------------------

        std::string response_data =
            response_builder.build(
                response
            );


        // -----------------------------------------------------
        // SEND RESPONSE
        // -----------------------------------------------------

        send_all(
            client_fd,
            response_data.c_str(),
            response_data.size()
        );


        // -----------------------------------------------------
        // CLOSE CLIENT
        // -----------------------------------------------------

        close(client_fd);


        std::cout
            << "Client disconnected\n";
    }


    // ---------------------------------------------------------
    // SHUTDOWN SERVER
    // ---------------------------------------------------------

    close(server_fd);

    return 0;
}





// -------------------------------------------------------------
// SEND ALL
// -------------------------------------------------------------

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


bool parse_content_length(const HttpRequest request, size_t& content_length) {

    auto header = request.headers.find("Content-Length");

    // No Content-Length means no body
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

        // Make sure the entire value was numeric
        if (position != header->second.size()) return false;

        content_length = static_cast<size_t>(value);    
    } catch (...) {
        return false;
    }

    return true;

}