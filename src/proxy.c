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
#include <pthread.h>
#include "proxy_thread.h"

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

        connection_ctx_t *ctx = malloc(sizeof(connection_ctx_t));
        if (ctx == NULL) {
            log_error("Failed to allocate memory for connection context");
            close(client_fd);
            continue;
        }
        ctx->client_fd = client_fd;
        strncpy(ctx->client_ip, client_ip, sizeof(ctx->client_ip) - 1);
        ctx->client_ip[sizeof(ctx->client_ip) - 1] = '\0';
        ctx->client_port = client_port;
        strncpy(ctx->target_host, config->target_host, sizeof(ctx->target_host) - 1);
        ctx->target_host[sizeof(ctx->target_host) - 1] = '\0';
        ctx->target_port = config->target_port;
        pthread_t thread;
        if ((err = pthread_create(&thread, NULL, proxy_thread_func, ctx)) != 0) {
            log_error("Failed to create thread: %s", strerror(err));
            close(client_fd);
            free(ctx);
            continue;
        }
        pthread_detach(thread);
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
