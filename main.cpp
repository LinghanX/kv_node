#include <iostream>
#include "include/spdlog/spdlog.h"
#include "include/spdlog/sinks/stdout_color_sinks.h"
#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "message.h"

void print_all(std::vector<std::string> tokens) {
    for (const auto& n : tokens) {
        std::cout << n << std::endl;
    }
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char **argv) {
    const auto logger = spdlog::stdout_color_mt("console");
    logger -> info("welcome to client");
    std::map<std::string, std::string> store;

    std::string address, port_num, user_input;
    std::tie(address, port_num) = handle_input(argc, argv);

    logger -> info("listenning on {}: {}", address, port_num);

    // --------- start connecting ------
    fd_set master;
    fd_set read_fds;
    int fdmax;

    int listener;
    int newfd;
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;

    char buf[1024];
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes = 1;
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ( (rv = getaddrinfo(nullptr, port_num.c_str(), &hints, &ai)) != 0 )
        logger -> error("unable to get addr");

    for (p = ai; p != nullptr; p = p -> ai_next) {
        listener = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol);

        if (listener < 0) continue;

        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if ( bind(listener, p -> ai_addr, p -> ai_addrlen) < 0 ) {
            close (listener);
            continue;
        }
        break;
    }

    if (p == nullptr) logger -> error("unable to bind");

    freeaddrinfo(ai);

    if ( listen(listener, 10) == -1 ) logger -> error("listen error");

    int incoming_fd;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    while (true) {
        sin_size = sizeof their_addr;
        if ( (incoming_fd = accept(listener, (struct sockaddr *)&their_addr, &sin_size)) == -1 )
            logger -> error("unable to select");

        if ( (nbytes = recv(incoming_fd, buf, sizeof(buf), 0)) <= 0) {
            if (nbytes == 0) {
                printf("client %d hung up\n", incoming_fd);
            } else {
                perror("recv");
            }
            close(incoming_fd);
        } else {
            // got message from a client
            msg_type type = check_msg_type(buf, nbytes);
            switch (type) {
                case msg_type::JOIN:
                {
                    join_msg* msg = (join_msg *) buf;

                    std::string addr(msg -> addr, 256);
                    std::string port(msg -> port, 256);

                    logger -> info("joining node addr: {}, port: {}", addr, port);

                    break;
                }
                case msg_type::GET:
                {
                    get_msg* msg = (get_msg *) buf;
                    std::string key(msg -> key, 256);
                    std::string value = store[key];
                    logger -> info("getting key: {}, value: {}", key, value);

                    if (send(incoming_fd, value.c_str(), value.length(), 0) < 0)
                        logger -> error("unable to send value");

                    break;
                }
                case msg_type::PUT:
                {
                    put_msg* msg = (put_msg *) buf;
                    std::string key(msg -> key, 256);
                    std::string value(msg -> value, 256);

                    logger -> info("storing key: {}, value: {}", key, value);

                    store[key] = value;
                    break;
                }
                case msg_type::KILL:
                {
                    kill_msg* msg = (kill_msg *) buf;
                    std::string addr(msg -> addr, 256);
                    std::string port(msg -> port, 256);

                    logger -> info("killing node addr: {}, port: {}", addr, port);
                    break;
                }
                case msg_type::ERROR:
                {
                    logger -> error("error message type");
                    break;
                }
                default:
                    logger -> error("should not be here");
            }
        }
    }
}
