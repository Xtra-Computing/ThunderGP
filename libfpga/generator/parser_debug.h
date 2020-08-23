#ifndef __PARSER_DEBUG__
#define __PARSER_DEBUG__


#define DEBUG_PRINTF(fmt,...)   logger((char *)fmt,##__VA_ARGS__);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) sizeof(arr)/sizeof((arr)[0])
#endif

extern void logger (char *fmt, ...);


#endif /* __PARSER_DEBUG__ */
