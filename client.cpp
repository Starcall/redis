#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include <string>
#include <iostream>

void panic(const std::string & message) {
    std::cerr << "client.cpp [ERROR] " << message << std::endl;
    exit(0);
}

void log(const std::string & message) {
    std::cerr << "client.cpp [DEBUG] " << message << std::endl;
}

int main (int argc, char *argv[]) {

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        panic("socket()");
    };

    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = ntohs(1234);
    address.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); //127.0.0.1
    
    int rc = connect(fd, (const sockaddr*)&address, sizeof(address));

    if (rc) {
        panic("connect()");
    }

    char msg[] = "ping";
    write(fd, msg, strlen(msg));

    char rbuf[64] = {};
    ssize_t n = read(fd, &rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
        panic("read()");
    } 
    log("server says: " + std::string(rbuf)); 
    close(fd);

    return 0;
}
