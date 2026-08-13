#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>


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

    return 0;
}