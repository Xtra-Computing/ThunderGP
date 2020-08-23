#include <string>
#include <stdarg.h>

void logger (char *fmt, ...)
{
    va_list argp;
    fprintf(stdout, "[PARSER] ");
    va_start(argp, fmt);
    vfprintf(stdout, fmt, argp);
    va_end(argp);
}
