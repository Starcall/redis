#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include <string>
#include <iostream>

void panic(const std::string & message) {
    std::cerr << "server.cpp [ERROR] " << message << std::endl;
    exit(0);
}

void log(const std::string & message) {
    std::cerr << "server.cpp [DEBUG] " << message << std::endl;
}

void do_something(int connected_fd) {
    char rbuf[64] = {};
    ssize_t n = read(connected_fd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
        log("read() error");
    }
    log("client says: " + std::string(rbuf));
    char wbuf[] = "pong";
    write(connected_fd, wbuf, strlen(wbuf));
}



int main (int argc, char *argv[]) {

    int fd = socket(AF_INET, SOCK_STREAM, 0); // IPv4 TCP
    
    int val;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    
    sockaddr_in address = {};
    address.sin_family = AF_INET; // IPv4
    address.sin_port = ntohs(1234); // port 1234
    address.sin_addr.s_addr = ntohl(0); // 0.0.0.0
    
    int rc = bind(fd, (const sockaddr*)&address, sizeof(address));

    if (rc) {
        panic("bind()");
    }

    rc = listen(fd, SOMAXCONN);
    
    if (rc) {
        panic("listen()");
    }

    while (true) {
        sockaddr_in client_address = {};
        socklen_t sock_len = sizeof(client_address);
        int connected_fd = accept(fd, (struct sockaddr*)&client_address, &sock_len);
        if (connected_fd < 0) {
            continue; // error
        }

        do_something(connected_fd);
        close(connected_fd);
    }

    return 0;
}
