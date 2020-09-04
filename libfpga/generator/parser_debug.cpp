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


bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}
