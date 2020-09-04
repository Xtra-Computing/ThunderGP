#ifndef __COMMON_H__
#define __COMMON_H__


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>


#define EXPAND(x)                   x


#define STRINGIFY_MACRO(x)          STR(x)
#define STR(x)                      #x

#define CONCAT4(n1, n2, n3, n4)     STRINGIFY_MACRO(EXPAND(n1)EXPAND(n2)EXPAND(n3)EXPAND(n4))

#define CAT_SECOND_LEVLE(x, y)      x ## y
#define VAR_CONCAT2(x, y)           CAT_SECOND_LEVLE(x, y)


#ifndef FLAG_SET
#define FLAG_SET                (1u)
#endif

#ifndef FLAG_RESET
#define FLAG_RESET              (0u)
#endif


#if 1

#define DEBUG_PRINTF(fmt,...)   printf(fmt,##__VA_ARGS__); fflush(stdout);

#else

#define DEBUG_PRINTF(fmt,...)   ;

#endif


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) sizeof(arr)/sizeof((arr)[0])
#endif


inline unsigned int get_aligned_size(unsigned int in, unsigned int align)
{
	if (in == 0)
	{
		return align;
	}
	else
	{
		return (((((in - 1) / align) + 1) * align));
	}
}

#define SIZE_ALIGNMENT(in,align)    get_aligned_size((unsigned int)in,(unsigned int)align)


#endif /* __COMMON_H__ */
