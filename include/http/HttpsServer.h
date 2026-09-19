#pragma once

#include <string>
#include <utility>

#include <openssl/ssl.h>

#include "http/Router.h"

namespace http {

class HttpsServer {

    public:

        HttpsServer(
            int port,
            Router router,
            const std::string& cert_path,
            const std::string& key_path
        );

        ~HttpsServer();

        void run();

    private:

        int server_fd;
        int port;
        Router router;
        SSL_CTX* ssl_context;

        void setup();          // sets up the listening socket
        void setup_ssl(
            const std::string& cert_path,
            const std::string& key_path
        );                     // sets up the SSL context
        void handle_client(int client_fd);
};

}  // namespace http