#include <stdexcept>
#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <algorithm>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "http/HttpRequest.h"
#include "http/HttpParser.h"
#include "http/HttpResponseBuilder.h"
#include "http/Router.h"
#include "http/HttpsServer.h"

namespace http {

// Free functions shared with the plaintext server live in HttpServer.cpp:
//   bool parse_content_length(const HttpRequest&, size_t&);
//   bool should_keep_alive(const HttpRequest&);
// Declare them so this file can use them too.
bool parse_content_length(const HttpRequest& request, size_t& content_length);
bool should_keep_alive(const HttpRequest& request);

constexpr size_t HTTPS_MAX_BODY_SIZE = 1024 * 1024;


HttpsServer::HttpsServer(
    int port,
    Router router,
    const std::string& cert_path,
    const std::string& key_path
)
    : server_fd(-1),
      port(port),
      router(std::move(router)),
      ssl_context(nullptr) {

    setup_ssl(cert_path, key_path);
    setup();
}


HttpsServer::~HttpsServer() {
    if (ssl_context) {
        SSL_CTX_free(ssl_context);
    }
    if (server_fd != -1) {
        close(server_fd);
    }
}


void HttpsServer::setup_ssl(
    const std::string& cert_path,
    const std::string& key_path
) {

    // Create a TLS server context using OpenSSL's flexible method,
    // which negotiates the best TLS version both sides support.
    ssl_context = SSL_CTX_new(TLS_server_method());

    if (!ssl_context) {
        throw std::runtime_error("Failed to create SSL context");
    }

    // Load the certificate chain (the public cert sent to clients).
    if (SSL_CTX_use_certificate_file(
            ssl_context,
            cert_path.c_str(),
            SSL_FILETYPE_PEM
        ) <= 0) {
        throw std::runtime_error(
            "Failed to load certificate: " + cert_path
        );
    }

    // Load the private key (kept secret on the server).
    if (SSL_CTX_use_PrivateKey_file(
            ssl_context,
            key_path.c_str(),
            SSL_FILETYPE_PEM
        ) <= 0) {
        throw std::runtime_error(
            "Failed to load private key: " + key_path
        );
    }

    // Sanity check: confirm the private key matches the certificate.
    if (!SSL_CTX_check_private_key(ssl_context)) {
        throw std::runtime_error(
            "Private key does not match the certificate"
        );
    }
}


// SETUP
void HttpsServer::setup() {

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        throw std::runtime_error("Failed to create socket");
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
        throw std::runtime_error("Failed to set SO_REUSEADDR");
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_address.sin_port = htons(port);

    if (bind(
            server_fd,
            (sockaddr*)&server_address,
            sizeof(server_address)
        ) == -1) {
        close(server_fd);
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(server_fd, 10) == -1) {
        close(server_fd);
        throw std::runtime_error("Failed to listen");
    }

    std::cout
        << "Server listening on "
        << "https://localhost:"
        << port
        << '\n';
}


// ---------------------------------------------
// RUN
// ---------------------------------------------
void HttpsServer::run() {

    while (true) {

        int client_fd = accept(server_fd, nullptr, nullptr);

        if (client_fd == -1) {
            std::cerr << "Failed to accept client\n";
            continue;
        }

        std::cout << "Client connected (TLS)\n";

        std::thread client_thread(
            &HttpsServer::handle_client,
            this,
            client_fd
        );

        client_thread.detach();
    }
}

// ----------------------------------------------------
// HANDLE CLIENT
// ----------------------------------------------------
void HttpsServer::handle_client(int client_fd) {

    // --- Wrap the raw socket in an SSL object and handshake ---
    SSL* ssl = SSL_new(ssl_context);

    if (!ssl) {
        std::cerr << "SSL_new failed\n";
        close(client_fd);
        return;
    }

    SSL_set_fd(ssl, client_fd);

    if (SSL_accept(ssl) <= 0) {
        // Handshake failed (bad cert, non-TLS client, etc.)
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(client_fd);
        return;
    }

    HttpParser parser;
    HttpResponseBuilder response_builder;

    char buffer[4096];

    while (true) {

        // --- Read until we have a full header section ---
        std::string raw_request;

        while (raw_request.find("\r\n\r\n") == std::string::npos) {

            int bytes_received = SSL_read(ssl, buffer, sizeof(buffer));

            if (bytes_received <= 0) {
                // 0 or negative: client closed or error. Normal end.
                SSL_shutdown(ssl);
                SSL_free(ssl);
                close(client_fd);
                return;
            }

            raw_request.append(buffer, bytes_received);
        }

        // --- Parse ---
        HttpRequest request;

        if (!parser.parse(raw_request, request)) {
            std::cerr << "Failed to parse HTTPS request\n";
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(client_fd);
            return;
        }

        // --- Read the body per Content-Length ---
        size_t content_length = 0;

        if (!parse_content_length(request, content_length)) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(client_fd);
            return;
        }

        if (content_length > HTTPS_MAX_BODY_SIZE) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(client_fd);
            return;
        }

        size_t body_bytes_received = request.body.size();

        while (body_bytes_received < content_length) {
            size_t remaining = content_length - body_bytes_received;
            size_t to_read = std::min(remaining, sizeof(buffer));

            int bytes_received = SSL_read(ssl, buffer, to_read);

            if (bytes_received <= 0) {
                SSL_shutdown(ssl);
                SSL_free(ssl);
                close(client_fd);
                return;
            }

            request.body.append(buffer, bytes_received);
            body_bytes_received += bytes_received;
        }

        request.parse_form_body();

        // --- Route and respond ---
        bool keep_alive = should_keep_alive(request);

        HttpResponse response = router.handle(request);

        response.headers["Connection"] = keep_alive ? "keep-alive" : "close";

        std::string response_data = response_builder.build(response);

        if (SSL_write(ssl, response_data.c_str(), response_data.size()) <= 0) {
            ERR_print_errors_fp(stderr);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(client_fd);
            return;
        }

        if (!keep_alive) {
            break;
        }
    }

    // --- Clean shutdown ---
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(client_fd);
    std::cout << "Client disconnected (TLS)\n";
}


}