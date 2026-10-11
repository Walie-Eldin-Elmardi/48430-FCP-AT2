#include <stdarg.h>
#include "custom_library.h"

void debug_print(const char *fmt, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "DEBUG: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
#else
    (void)fmt; /* No debug output in release mode */
#endif
}
