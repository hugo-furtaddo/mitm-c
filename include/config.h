#ifndef MITM_C_CONFIG_H
#define MITM_C_CONFIG_H

#include <stdint.h>

typedef struct {
    char listen_host[256];
    uint16_t listen_port;

    char target_host[256];
    uint16_t target_port;

#ifdef ENABLE_UDP
    int enable_udp;
#endif

#ifdef ENABLE_TLS
    int enable_tls;
#endif

} proxy_config_t;

int config_load(const char *filepath, proxy_config_t *config);
void free_config(proxy_config_t *config);

#endif /* MITM_C_CONFIG_H */
