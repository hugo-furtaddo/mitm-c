#include <stdio.h>
#include "ui.h"

void display_menu(int is_proxy_running) {
    printf("\n--- mitm-c Proxy Menu ---\n");
    printf("Status: %s\n", is_proxy_running ? "Running" : "Stopped");
    printf("1. View Configuration\n");
    printf("2. Start Proxy\n");
    printf("3. Stop Proxy\n");
    printf("4. Show Usage Explanation\n");
    printf("5. Exit\n");
    printf("Enter your choice: ");
}

void display_usage_explanation() {
    printf("\n--- Usage Explanation ---\n");
    printf("This is a modular reverse proxy (mitm-c).\n");
    printf(" - The proxy's behavior is defined in 'proxy.conf'.\n");
    printf(" - You can start or stop the proxy using the menu.\n");
    printf(" - View the current configuration to see how it's set up.\n");
    printf(" - The proxy will intercept TCP traffic based on the configuration.\n");
    printf("--------------------------\n");
}

void display_current_config(proxy_config_t *config) {
    printf("\n--- Current Configuration ---\n");
    printf("Listen Host: %s\n", config->listen_host);
    printf("Listen Port: %d\n", config->listen_port);
    printf("Target Host: %s\n", config->target_host);
    printf("Target Port: %d\n", config->target_port);
    printf("---------------------------\n");
