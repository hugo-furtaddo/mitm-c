#ifndef MITM_C_PROXY_H
#define MITM_C_PROXY_H

#include "config.h"
#include "modules.h"

int start_proxy(const proxy_config_t *config);
void stop_proxy(void);

#endif /* MITM_C_PROXY_H */
