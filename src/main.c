#include <stdlib.h>

#include "config.h"
#include "logger_module.h"
#include "modules.h"
#include "proxy.h"
#include "utils.h"

int main(void) {
  proxy_config_t config;
  if (config_load("config/proxy.conf", &config) != 0) return EXIT_FAILURE;

  if (proxy_register_module(register_logger_module()) != 0) return EXIT_FAILURE;

  if (start_proxy(&config) != 0) {
    stop_proxy();
    return EXIT_FAILURE;
  }

  stop_proxy();
  return EXIT_SUCCESS;
}
