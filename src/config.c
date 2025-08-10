#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int config_load(const char *filename, proxy_config_t *config) {
  FILE *f = fopen(filename, "r");
  if (!f) {
    log_error("Não foi possível abrir arquivo de configuração: %s", filename);
    return -1;
  }
  char line[512];
  int fields_set = 0;
  while (fgets(line, sizeof(line), f)) {
    size_t len = strlen(line);
    if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
      while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[--len] = '\0';
      }
    }
    // Skip empty or comment lines
    char *ptr = line;
    while (isspace((unsigned char)*ptr)) ptr++;
    if (*ptr == '\0' || *ptr == '#') {
      continue;
    }
    char *eq = strchr(ptr, '=');
    if (!eq) {
      continue;
    }
    *eq = '\0';
    char *key = ptr;
    char *value = eq + 1;
    // Trim whitespace from key and value
    char *end = key + strlen(key) - 1;
    while (end >= key && isspace((unsigned char)*end)) {
      *end = '\0';
      end--;
    }
    while (isspace((unsigned char)*value)) value++;
    end = value + strlen(value) - 1;
    while (end >= value && isspace((unsigned char)*end)) {
      *end = '\0';
      end--;
    }
    if (strcmp(key, "listen_host") == 0) {
      strncpy(config->listen_host, value, sizeof(config->listen_host) - 1);
      config->listen_host[sizeof(config->listen_host) - 1] = '\0';
      fields_set++;
    } else if (strcmp(key, "listen_port") == 0) {
      config->listen_port = atoi(value);
      fields_set++;
    } else if (strcmp(key, "target_host") == 0) {
      strncpy(config->target_host, value, sizeof(config->target_host) - 1);
      config->target_host[sizeof(config->target_host) - 1] = '\0';
      fields_set++;
    } else if (strcmp(key, "target_port") == 0) {
      config->target_port = atoi(value);
      fields_set++;
    }
    // ignore unknown keys
  }
  fclose(f);
  if (fields_set < 4) {
    log_error("Arquivo de configuração incompleto ou inválido");
    return -1;
  }
  return 0;
}
