#include <stdio.h>
#include <string.h>
#include "modules.h"
#include "utils.h"

static const proxy_module_t *registered_module = NULL;

int proxy_register_module(const proxy_module_t *module) {
    if (registered_module != NULL) {
        log_error("Only one module can be registered at a time.");
        return -1;
    }
    registered_module = module;
    return 0;
}

int proxy_unregister_module(const char *module_name) {
    if (registered_module && registered_module->name &&
        strcmp(registered_module->name, module_name) == 0) {
        registered_module = NULL;
        return 0;
    }
    log_warn("Module '%s' is not registered.", module_name);
    return -1;
}

const proxy_module_t *proxy_get_module(const char *module_name) {
    if (registered_module && registered_module->name &&
        strcmp(registered_module->name, module_name) == 0) {
        return registered_module;
    }
    return NULL;
}

void modules_on_connect(const char *ip, int port) {
    if (registered_module && registered_module->on_connect) {
        registered_module->on_connect(ip, port);
    }
}

void modules_on_disconnect(const char *ip, int port) {
    if (registered_module && registered_module->on_disconnect) {
        registered_module->on_disconnect(ip, port);
    }
}

void modules_on_data(int direction, size_t bytes) {
    if (registered_module && registered_module->on_data) {
        registered_module->on_data(direction, bytes);
    }
}
