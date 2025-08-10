#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "modules.h"
#include "logger_module.h"
#include "proxy.h"
#include "utils.h"


int main(void) {
    proxy_config_t config;
    if (config_load("proxy.conf", &config) != 0)
        return EXIT_FAILURE;

    if (proxy_register_module(register_logger_module()) != 0)
        return EXIT_FAILURE;

    int is_proxy_running = 0;
    int choice = 0;

    while (1) {
        printf("
    }

    stop_proxy();
    return EXIT_SUCCESS;
}

