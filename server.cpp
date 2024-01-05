#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include "protocol.h"
#include "utils.h"

utils::Logger logger("server.cpp");

class Parser {
private:
    char rbuf[protocol::HEADER_SIZE + protocol::MAX_MESSAGE_SIZE + 1];
    char wbuf[protocol::HEADER_SIZE + protocol::MAX_MESSAGE_SIZE + 1];
public:
    int32_t parse_request(int connected_fd) {
        logger.log("parse_request");
        memset(rbuf, 1, sizeof(rbuf));
        errno = 0;
        int32_t rc = utils::IO::read_full(connected_fd, rbuf, protocol::HEADER_SIZE);
        if (rc) {
            if (errno == 0) {
                logger.log("EOF");
            } else {
                logger.log("failed to read()");
            }
            return rc;
        }
        uint32_t message_len = 0;
        memcpy(&message_len, rbuf, protocol::HEADER_SIZE);
        if (message_len > protocol::MAX_MESSAGE_SIZE) {
            logger.log("Message too long! Message size: " + std::to_string(message_len));
            return -1;
        }
        errno = 0;
        rc = utils::IO::read_full(connected_fd, &rbuf[protocol::HEADER_SIZE], message_len);
        if (rc) {
            if (errno == 0) {
                logger.log("Unexpected EOF");
            } else {
                logger.log("Failed to read()");
            }
            return rc;
        }
        rbuf[protocol::HEADER_SIZE + message_len] = '\0';
        logger.log("client says: " + std::string(&rbuf[protocol::HEADER_SIZE]));

        memset(wbuf, 1, sizeof(wbuf));
        const char reply[] = "pong";
        uint32_t reply_message_len = (uint32_t)strlen(reply);
        memcpy(wbuf, &reply_message_len, protocol::HEADER_SIZE);
        memcpy(&wbuf[protocol::HEADER_SIZE], reply, reply_message_len); 
        return utils::IO::write_full(connected_fd, wbuf, protocol::HEADER_SIZE + reply_message_len);
    }

};


int main(int argc, char *argv[]) {

    int fd = socket(AF_INET, SOCK_STREAM, 0); // IPv4 TCP
    
    int val;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    
    sockaddr_in address = {};
    address.sin_family = AF_INET; // IPv4
    address.sin_port = ntohs(1234); // port 1234
    address.sin_addr.s_addr = ntohl(0); // 0.0.0.0
    
    int rc = bind(fd, (const sockaddr*)&address, sizeof(address));

    if (rc) {
        logger.panic("bind()");
    }

    rc = listen(fd, SOMAXCONN);
    
    if (rc) {
        logger.panic("listen()");
    }

    Parser request_parser;

    while (true) {
        sockaddr_in client_address = {};
        socklen_t sock_len = sizeof(client_address);
        int connected_fd = accept(fd, (struct sockaddr*)&client_address, &sock_len);
        if (connected_fd < 0) {
            continue; // error
        }

        while (true) {
            int32_t err = request_parser.parse_request(connected_fd);
            if (err) {
                break;
            }
        }
        close(connected_fd);
    }

    return 0;
}
