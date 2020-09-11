#ifndef __PARSER_DEBUG__
#define __PARSER_DEBUG__

#undef DEBUG_PRINTF
#define DEBUG_PRINTF(fmt,...)   logger((char *)fmt,##__VA_ARGS__);


extern void logger (char *fmt, ...);


#endif /* __PARSER_DEBUG__ */
