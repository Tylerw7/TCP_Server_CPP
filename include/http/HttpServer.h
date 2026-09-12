#pragma once

#include "http/Router.h"
#include <utility>

namespace http {

    class HttpServer {

    public:

        HttpServer(int port, Router router);

        void run();

    private:

        int server_fd;
        int port;
        Router router;

        void setup();
        void handle_client(int client_fd);    
};

}

