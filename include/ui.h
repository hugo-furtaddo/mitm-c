#ifndef UI_H
#define UI_H

#include "config.h"

void display_menu(int is_proxy_running);
void display_usage_explanation();
void display_current_config(proxy_config_t *config);

#endif