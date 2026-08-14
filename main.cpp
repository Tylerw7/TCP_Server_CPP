#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <sstream>


int main() {

    // Whats Happening?
    // - AF_INET: tells the OS "I want to use IPv4" EX: 127.0.0.1
    // - SOCK_STREAM: tells OS "I want a stream-oriented socket"
    // TCP gives us a reliable, ordered byte stream. TCP is responsible for making sure the other side receives those bytes reliably and in order. 
    // 0: "Use the default protocol for this socket type."
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Address structure
    sockaddr_in server_address{};

    // Set the address family
    server_address.sin_family = AF_INET;

    // Set the IP Address
    // - htons() means: Host-To-Network-Short
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // Set the Port
    server_address.sin_port = htons(8080);

    int result = bind(
        server_fd,
        (sockaddr*)&server_address,
        sizeof(server_address)
    );

    std::cout << "Bind result: " << result << "\n";

    result = listen(server_fd,10);

    std::cout << "Listen result: " << result << "\n";

    int client_fd = accept(server_fd, nullptr, nullptr);

    std::cout << "Client connected!\n";
    std::cout << "Client socket: " << client_fd << "\n";

    char buffer [4096]; // array of 4096 characters (bytes) in memory
    int bytes_recieved = recv(
        client_fd,
        buffer,
        sizeof(buffer),
        0
    );

    //Parse HTTP request

    std::string request(buffer, bytes_recieved); // To get the length
    size_t end_of_line = request.find("\r\n");
    std::string request_line = request.substr(0, end_of_line);
    std::istringstream stream(request_line);

    std::string method;
    std::string path;
    std::string version;

    stream >> method >> path >> version;

    std::cout << "Method: " << method << '\n';
    std::cout << "Path: " << path << '\n';
    std::cout << "Version: " << version << '\n';

    if (method == "GET" && path == "/") {
        const char* response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 12\r\n"
            "\r\n"
            "New Message 1";

        send(client_fd, response, strlen(response),0);    
    }

    const char* response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 12\r\n"
        "\r\n"
        "Hello world!";

    send(client_fd, response, strlen(response),0);    


    std::cout << "Bytes recieved: " << bytes_recieved << "\n";
    std::cout.write(buffer, bytes_recieved);

    return 0;
}