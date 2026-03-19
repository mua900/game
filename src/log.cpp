#include "log.h"

// @todo custom formatting
// @todo log colors

#include <cstdio>

void log_info(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[256];

    va_list forward_args;
    va_copy(forward_args, args);

    int written = snprintf(buffer, 256, "INFO: ");
    vsnprintf(buffer + written, 256 - written, format, forward_args);

    fprintf(stdout, "%s\n", buffer);

    va_end(args);
}

void log_warning(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[256];

    va_list forward_args;
    va_copy(forward_args, args);

    int written = snprintf(buffer, 256, "WARNING: ");
    vsnprintf(buffer + written, 256 - written, format, forward_args);

    fprintf(stderr, "%s\n", buffer);

    va_end(args);
}

void log_error(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[256];

    va_list forward_args;
    va_copy(forward_args, args);

    int written = snprintf(buffer, 256, "ERROR: ");
    vsnprintf(buffer + written, 256 - written, format, forward_args);

    fprintf(stderr, "%s\n", buffer);

    va_end(args);
}
