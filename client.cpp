#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include "protocol.h"
#include "utils.h"

utils::Logger logger("client.cpp");

class ServerConnector {
private:
    char rbuf[protocol::HEADER_SIZE + protocol::MAX_MESSAGE_SIZE + 1];
    char wbuf[protocol::HEADER_SIZE + protocol::MAX_MESSAGE_SIZE + 1];
public:
    int32_t send_query(int fd, const char* text) {
        memset(wbuf, 1, sizeof(wbuf));
        uint32_t text_len = (uint32_t)strlen(text);
        if (text_len > protocol::MAX_MESSAGE_SIZE) {
            return -1;
        }
        memcpy(wbuf, &text_len, protocol::HEADER_SIZE);
        memcpy(&wbuf[protocol::HEADER_SIZE], text, text_len);
        if (int32_t rc = utils::IO::write_full(fd, wbuf, protocol::HEADER_SIZE + text_len)) {
            return rc;
        } 
        memset(rbuf, 1, sizeof(rbuf));
        errno = 0;
        int32_t rc = utils::IO::read_full(fd, rbuf, protocol::HEADER_SIZE);
        if (rc) {
            if (errno == 0) {
                logger.log("Unexpected EOF");
            } else {
                logger.log("Failed to read()");
            }
            return rc;
        }
        int32_t reply_message_len = 0;
        memcpy(&reply_message_len, rbuf, protocol::HEADER_SIZE);
        if ((size_t)reply_message_len > protocol::MAX_MESSAGE_SIZE) {
            logger.log("Message too long");
            return -1;
        }
        errno = 0;
        rc = utils::IO::read_full(fd, &rbuf[protocol::HEADER_SIZE], reply_message_len);
        if (rc) {
            if (errno == 0) {
                logger.log("Unexpected EOF");
            } else {
                logger.log("Failed to read()");
            }
            return rc;
        }
        rbuf[protocol::HEADER_SIZE + reply_message_len] = '\0';
        logger.log("Server replied with: " + std::string(&rbuf[protocol::HEADER_SIZE]));
        return 0;
    }
};

int main(int argc, char *argv[]) {

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        logger.panic("socket()");
    };

    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = ntohs(1234);
    address.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); //127.0.0.1
    
    int rc = connect(fd, (const sockaddr*)&address, sizeof(address));

    if (rc) {
        logger.panic("connect()");
    }

    ServerConnector server_connector;

    rc = server_connector.send_query(fd, "baba_zhaba_1");
    if (rc) {
        return 0;
    }
    rc = server_connector.send_query(fd, "baba_zhaba_2");
    if (rc) {
        return 0;
    }
    rc = server_connector.send_query(fd, "baba_zhaba_3");
    if (rc) {
        return 0;
    }

    close(fd);

    return 0;
}
