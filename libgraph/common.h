#ifndef __COMMON_H__
#define __COMMON_H__


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "config.h"



#define FLAG_SET                (1u)
#define FLAG_RESET              (0u)


#if 1

#define DEBUG_PRINTF(fmt,...)   printf(fmt,##__VA_ARGS__);

#else

#define DEBUG_PRINTF(fmt,...)   ;

#endif


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) sizeof(arr)/sizeof((arr)[0])
#endif


#define SIZE_ALIGNMENT(in,align)     ((((((unsigned int)(in + 1))/(align)) + 1) * (align )) - 1) 


#endif /* __COMMON_H__ */
