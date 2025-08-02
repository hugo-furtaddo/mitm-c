#ifndef MITM_C_MODULES_H
#define MITM_C_MODULES_H

#include <stddef.h>

typedef struct proxy_connection {
    int client_fd;
    int backend_fd;
} proxy_connection_t;

typedef void (*on_connect_cb)(proxy_connection_t *conn);
typedef void (*on_disconnect_cb)(proxy_connection_t *conn);
typedef void (*on_data_cb)(proxy_connection_t *conn, unsigned char *buffer, size_t len);

typedef struct proxy_module {
    const char *name;
    on_connect_cb on_connect;
    on_data_cb on_data;
    on_disconnect_cb on_disconnect;
} proxy_module_t;

int proxy_register_module(const proxy_module_t *module);

int proxy_unregister_module(const char *module_name);

const proxy_module_t *proxy_get_module(const char *module_name);

#endif /* MITM_C_MODULES_H */
