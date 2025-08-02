#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "config.h"
#include "modules.h"
#include "utils.h"


static int listen_fd = -1;
static int proxy_running = 0;

int start_proxy(const proxy_config_t *config) {
    struct addrinfo hints, *res;
    char port_str[16];
    int err, sockfd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    snprintf(port_str, sizeof(port_str), "%d", config->listen_port);
    if ((err = getaddrinfo(config->listen_host, port_str, &hints, &res)) != 0) {
        log_error("Failed to resolve or bind listening address: %s", gai_strerror(err));
        return -1;
    }

    struct addrinfo *ai;
    int opt = 1;
    for (ai = res; ai != NULL; ai = ai->ai_next) {
        sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (sockfd < 0) continue;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if (bind(sockfd, ai->ai_addr, ai->ai_addrlen) == 0) {
            break;
        }
        close(sockfd);
        sockfd = -1;
    }
    freeaddrinfo(res);

    if (sockfd < 0) {
        log_error("Unable to bind listening socket.");
        return -1;
    }

    if (listen(sockfd, 5) < 0) {
        log_error("Failed to listen on port %d", config->listen_port);
        close(sockfd);
        return -1;
    }

    listen_fd = sockfd;
    proxy_running = 1;

    while (proxy_running) {
        struct sockaddr_storage client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            if (!proxy_running) break;
            if (errno == EINTR) continue;
            log_error("Error accepting connection: %s", strerror(errno));
            break;
        }

        char client_ip[INET6_ADDRSTRLEN];
        int client_port;
        if (client_addr.ss_family == AF_INET) {
            struct sockaddr_in *s = (struct sockaddr_in *)&client_addr;
            inet_ntop(AF_INET, &s->sin_addr, client_ip, sizeof(client_ip));
            client_port = ntohs(s->sin_port);
        } else if (client_addr.ss_family == AF_INET6) {
            struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)&client_addr;
            inet_ntop(AF_INET6, &s6->sin6_addr, client_ip, sizeof(client_ip));
            client_port = ntohs(s6->sin6_port);
        } else {
            strcpy(client_ip, "Unknown");
            client_port = 0;
        }

        modules_on_connect(client_ip, client_port);

        int backend_fd = -1;
        struct addrinfo hints2, *res2;
        memset(&hints2, 0, sizeof(hints2));
        hints2.ai_family = AF_UNSPEC;
        hints2.ai_socktype = SOCK_STREAM;
        snprintf(port_str, sizeof(port_str), "%d", config->target_port);

        if ((err = getaddrinfo(config->target_host, port_str, &hints2, &res2)) != 0) {
            log_error("Failed to resolve backend: %s", gai_strerror(err));
        } else {
            struct addrinfo *ai2;
            for (ai2 = res2; ai2 != NULL; ai2 = ai2->ai_next) {
                backend_fd = socket(ai2->ai_family, ai2->ai_socktype, ai2->ai_protocol);
                if (backend_fd < 0) continue;
                if (connect(backend_fd, ai2->ai_addr, ai2->ai_addrlen) == 0) break;
                close(backend_fd);
                backend_fd = -1;
            }
            freeaddrinfo(res2);
        }

        if (backend_fd < 0) {
            log_error("Could not connect to backend %s:%d (DNS ou connect falhou)", config->target_host, config->target_port);
            modules_on_disconnect(client_ip, client_port);
            close(client_fd);
            continue;
        }

        fd_set readfds;
        int maxfd = (client_fd > backend_fd ? client_fd : backend_fd);
        ssize_t n;
        char buf[4096];

        while (1) {
            FD_ZERO(&readfds);
            FD_SET(client_fd, &readfds);
            FD_SET(backend_fd, &readfds);

            if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
                if (errno == EINTR) continue;
                log_error("Select error: %s", strerror(errno));
                break;
            }

            if (FD_ISSET(client_fd, &readfds)) {
                n = recv(client_fd, buf, sizeof(buf), 0);
                if (n <= 0) {
                    if (n < 0) log_error("Receive error from client: %s", strerror(errno));
                    break;
                }

                modules_on_data(MODULE_DATA_TO_BACKEND, (size_t)n);

                ssize_t tosend = n;
                char *p = buf;
                while (tosend > 0) {
                    ssize_t sent = send(backend_fd, p, tosend, 0);
                    if (sent < 0) {
                        log_error("Send error to backend: %s", strerror(errno));
                        break;
                    }
                    tosend -= sent;
                    p += sent;
                }
                if (tosend > 0) break;
            }

            if (FD_ISSET(backend_fd, &readfds)) {
                n = recv(backend_fd, buf, sizeof(buf), 0);
                if (n <= 0) {
                    if (n < 0) log_error("Receive error from backend: %s", strerror(errno));
                    break;
                }

                modules_on_data(MODULE_DATA_FROM_BACKEND, (size_t)n);

                ssize_t tosend = n;
                char *p = buf;
                while (tosend > 0) {
                    ssize_t sent = send(client_fd, p, tosend, 0);
                    if (sent < 0) {
                        log_error("Send error to client: %s", strerror(errno));
                        break;
                    }
                    tosend -= sent;
                    p += sent;
                }
                if (tosend > 0) break;
            }
        }

        close(backend_fd);
        modules_on_disconnect(client_ip, client_port);
        close(client_fd);
    }

    close(listen_fd);
    listen_fd = -1;
    proxy_running = 0;
    return 0;
}

int stop_proxy(void) {
    proxy_running = 0;
    if (listen_fd != -1) {
        close(listen_fd);
        listen_fd = -1;
    }
    return 0;
}
