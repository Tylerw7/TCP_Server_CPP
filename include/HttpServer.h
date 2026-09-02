#pragma once

class HttpServer {

    public:

        HttpServer(int port);

        void run();

    private:

        int server_fd;
        int port;

        void setup();
        void handle_client(int client_fd);    
};