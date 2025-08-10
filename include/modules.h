#ifndef MITM_C_MODULES_H
#define MITM_C_MODULES_H

#include <stddef.h>

#define MODULE_DATA_TO_BACKEND 0
#define MODULE_DATA_FROM_BACKEND 1

typedef void (*on_connect_cb)(const char *client_ip, int client_port);
typedef void (*on_disconnect_cb)(const char *client_ip, int client_port);
typedef void (*on_data_cb)(int direction, size_t bytes);

typedef struct proxy_module {
  const char *name;
  on_connect_cb on_connect;
  on_data_cb on_data;
  on_disconnect_cb on_disconnect;
} proxy_module_t;

int proxy_register_module(const proxy_module_t *module);
int proxy_unregister_module(const char *module_name);
const proxy_module_t *proxy_get_module(const char *module_name);

void modules_on_connect(const char *ip, int port);
void modules_on_disconnect(const char *ip, int port);
void modules_on_data(int direction, size_t bytes);

#endif /* MITM_C_MODULES_H */
