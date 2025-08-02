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
#include <pthread.h>

#include "utils.h"
#include "modules.h"
#include "proxy_thread.h"

void handle_client(connection_ctx_t *ctx) {
    // Notifica os módulos sobre a nova conexão
    modules_on_connect(ctx->client_ip, ctx->client_port);

    // Resolve e conecta ao servidor de destino (backend)
    struct addrinfo hints, *res;
    char port_str[16];
    int err;
    int backend_fd = -1;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    snprintf(port_str, sizeof(port_str), "%d", ctx->target_port);
    if ((err = getaddrinfo(ctx->target_host, port_str, &hints, &res)) != 0) {
        log_error("Failed to resolve backend: %s", gai_strerror(err));
    } else {
        for (struct addrinfo *ai = res; ai != NULL; ai = ai->ai_next) {
            backend_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
            if (backend_fd < 0) {
                continue;
            }
            if (connect(backend_fd, ai->ai_addr, ai->ai_addrlen) == 0) {
                break;
            }
            close(backend_fd);
            backend_fd = -1;
        }
        freeaddrinfo(res);
    }
    if (backend_fd < 0) {
        log_error("Could not connect to backend %s:%d", ctx->target_host, ctx->target_port);
        modules_on_disconnect(ctx->client_ip, ctx->client_port);
        close(ctx->client_fd);
        return;
    }

    // Loop de encaminhamento de dados entre client e backend
    fd_set readfds;
    int maxfd = (ctx->client_fd > backend_fd ? ctx->client_fd : backend_fd);
    char buf[4096];
    ssize_t n;
    for (;;) {
        FD_ZERO(&readfds);
        FD_SET(ctx->client_fd, &readfds);
        FD_SET(backend_fd, &readfds);
        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) {
                continue;
            }
            log_error("Select error: %s", strerror(errno));
            break;
        }
        if (FD_ISSET(ctx->client_fd, &readfds)) {
            n = recv(ctx->client_fd, buf, sizeof(buf), 0);
            if (n <= 0) {
                if (n < 0) {
                    log_error("Receive error from client: %s", strerror(errno));
                }
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
            if (tosend > 0) {
                break;
            }
        }
        if (FD_ISSET(backend_fd, &readfds)) {
            n = recv(backend_fd, buf, sizeof(buf), 0);
            if (n <= 0) {
                if (n < 0) {
                    log_error("Receive error from backend: %s", strerror(errno));
                }
                break;
            }
            modules_on_data(MODULE_DATA_FROM_BACKEND, (size_t)n);
            ssize_t tosend = n;
            char *p = buf;
            while (tosend > 0) {
                ssize_t sent = send(ctx->client_fd, p, tosend, 0);
                if (sent < 0) {
                    log_error("Send error to client: %s", strerror(errno));
                    break;
                }
                tosend -= sent;
                p += sent;
            }
            if (tosend > 0) {
                break;
            }
        }
    }

    // Limpeza após término da conexão
    close(backend_fd);
    modules_on_disconnect(ctx->client_ip, ctx->client_port);
    close(ctx->client_fd);
}

void *proxy_thread_func(void *arg) {
    connection_ctx_t *ctx = (connection_ctx_t *)arg;
    handle_client(ctx);
    free(ctx);
    return NULL;
}