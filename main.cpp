#include <iostream>
#include <sys/socket.h>


int main() {

    int server_id = socket(AF_INET, SOCK_STREAM, 0);

    std::cout << "Socket: " << server_id << "\n";
    
    return 0;
}