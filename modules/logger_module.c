#include <stdio.h>

#include "modules.h"
#include "utils.h"

static void on_connect_log(const char *client_ip, int client_port) {
  log_info("Connection to %s:%d", client_ip, client_port);
}

static void on_disconnect_log(const char *client_ip, int client_port) {
  log_info("Connection closed %s:%d", client_ip, client_port);
}

static void on_data_log(int direction, size_t bytes) {
  if (direction == MODULE_DATA_TO_BACKEND) {
    log_data("%zu bytes → backend", bytes);
  } else if (direction == MODULE_DATA_FROM_BACKEND) {
    log_data("%zu bytes ← backend", bytes);
  }
}

static proxy_module_t logger_module = {.name = "logger",
                                       .on_connect = on_connect_log,
                                       .on_disconnect = on_disconnect_log,
                                       .on_data = on_data_log};

proxy_module_t *register_logger_module(void) { return &logger_module; }
