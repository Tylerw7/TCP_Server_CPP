#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <csignal>

#include <sys/socket.h>
#include <netinet/in.h>


#include "HttpRequest.h"
#include "HttpParser.h"
#include "HttpResponseBuilder.h"
#include "Router.h"


// Global Variables
volatile sig_atomic_t running = true;


// Signal Handler
void handle_signal(int signal) {
    running = false;
}


// Declared functions
bool send_all(int socket_fd, const char* data, size_t length);



int main() {
    
    std::signal(SIGINT, handle_signal);

    // What's Happening?
    // - AF_INET: tells the OS "I want to use IPv4" EX: 127.0.0.1
    // - SOCK_STREAM: tells OS "I want a stream-oriented socket"
    // - TCP gives us a reliable, ordered byte stream.
    // - 0: "Use the default protocol for this socket type."

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    // SO_REUSEADDR
    int opt = 1;
    int result = setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

    if (result == -1) {
    std::cerr << "Failed to set SO_REUSEADDR\n";
    return 1;
    }

    // Address structure
    sockaddr_in server_address{};

    // Set the address family
    server_address.sin_family = AF_INET;

    // Set the IP Address
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // Set the Port
    server_address.sin_port = htons(8080);


    // Bind socket to IP + port
    result = bind(
        server_fd,
        (sockaddr*)&server_address,
        sizeof(server_address)
    );

    if (result == -1) {
        std::cerr << "Failed to bind socket\n";
        return 1;
    }

    // Start listening
    result = listen(server_fd, 10);

    if (result == -1) {
        std::cerr << "Failed to listen on socket\n";
        return 1;
    }

    std::cout << "Server listening on http://localhost:8080\n";

    // ------------------------------------
    // HTTPPARSER & Builder
    // ------------------------------------
    HttpParser parser;
    HttpResponseBuilder response_builder;


    // test routes
    Router router;

    router.get("/", []() {

        HttpResponse response;

        response.status = HttpStatus::OK;
        response.headers["Content-Type"] = "text/plain";
        response.body = "Welcome to Tylers server from '/'";

        return response;
    });


    router.get("/hello", []() {

        HttpResponse response;

        response.status = HttpStatus::OK;
        response.headers["Content-Type"] = "text/plain";
        response.body = "Hello from the /hello route!";

        return response;
    });

    

    // Keep accepting clients
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

            std::cerr << "Failed to accept client\n";
            continue;
        }

        std::cout << "Client connected!\n";
        std::cout << "Client socket: " << client_fd << "\n";

        // Buffer for receiving TCP data
        char buffer[4096];

        // Store the complete HTTP request
        std::string raw_request;

        // Keep receiving until we've received
        // the end of the HTTP headers.
        // ------ RECIEVE LOOP ------
        while (raw_request.find("\r\n\r\n") == std::string::npos) {


            ssize_t bytes_received = recv(
                client_fd,
                buffer,
                sizeof(buffer),
                0
            );

            if (bytes_received == 0) {
                std::cout << "Client closed connection\n";
                break;
            }

            if (bytes_received == -1) {
                std::cerr << "recv() failed: " << strerror(errno) << '\n';
                break;
            }

            raw_request.append(buffer, bytes_received);
        }
        HttpRequest request;

        // Parse HTTP request ------------------------------
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
        // DISPLAY REQUEST
        // -----------------------------------------------------

        std::cout
            << "Method: "
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


        std::cout << "\nHeaders:\n";


        for (const auto& header : request.headers) {

            std::cout
                << header.first
                << " = "
                << header.second
                << '\n';
        }


        // -----------------------------------------------------
        // TEMPORARY HTTP RESPONSE
        // -----------------------------------------------------

        HttpResponse response =
            router.handle(
                request.method,
                request.path
            );

        // Build Response
        std::string response_data = response_builder.build(response);


        send_all(
            client_fd,
            response_data.c_str(),
            response_data.size()
        );


        // -----------------------------------------------------
        // CLOSE CLIENT CONNECTION
        // -----------------------------------------------------

        close(client_fd);

        std::cout
            << "Client disconnected\n";
    }

    close(server_fd);
    

    return 0;
}


// ------------------------------------------------------------------------------
// END OF MAIN FUNCTION
// START OF HELPER FUNCTIONS
// ------------------------------------------------------------------------------


bool send_all(int socket_fd, const char* data, size_t length) {
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