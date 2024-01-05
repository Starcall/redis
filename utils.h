#pragma once

#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <netinet/ip.h>
#include <iostream>

namespace utils
{
class Logger {
public:
    Logger(const std::string &filename) {
        m_filename = filename;
    }
    void panic(const std::string & message) {
        std::cerr << m_filename << " [ERROR] " << message << std::endl;
        exit(0);
    }

    void log(const std::string & message) {
        std::cerr << m_filename << " [DEBUG] " << message << std::endl;
    }
    std::string m_filename;
};

class IO {
public:
    static int32_t read_full(int fd, char* buf, size_t n) {
        while (n > 0) {
            ssize_t return_value = read(fd, buf, n);
            if (return_value <= 0) {
                return -1;
            }
            assert((size_t)return_value <= n);
            buf += (size_t)return_value;
            n -= return_value;
        }
        return 0;
    }

    static int32_t write_full(int fd, const char* buf, size_t n) {
        while (n > 0) {
            ssize_t return_value = write(fd, buf, n);
            if (return_value <= 0) {
                return -1;
            }
            assert((size_t)return_value <= n);
            buf += return_value;
            n -= return_value;
        }
        return 0;
    }

};
}
