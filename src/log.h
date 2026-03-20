#ifndef _LOG_H
#define _LOG_H

#include <cstdarg>

#define LOG_VERBOSE 0

void log_info(const char* format, ...);
void log_warning(const char* format, ...);
void log_error(const char* format, ...);

#endif // _LOG_H