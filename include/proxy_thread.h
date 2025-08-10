#ifndef MITM_C_PROXY_THREAD_H
#define MITM_C_PROXY_THREAD_H

#include <netinet/in.h>

#include "config.h"

typedef struct {
  int client_fd;
  char client_ip[INET6_ADDRSTRLEN];
  int client_port;
  char target_host[256];
  uint16_t target_port;
} connection_ctx_t;

void *proxy_thread_func(void *arg);
void handle_client(connection_ctx_t *ctx);

#endif /* MITM_C_PROXY_THREAD_H */
