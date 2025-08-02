#ifndef MITM_C_UTILS_H
#define MITM_C_UTILS_H

#include <stddef.h>

void log_debug(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_error(const char *fmt, ...);

char *read_file_to_buffer(const char *filepath);

char *trim_whitespace(char *str);

#endif /* MITM_C_UTILS_H */
