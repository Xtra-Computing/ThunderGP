#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <climits>
#include <math.h>


#define MAX_ITER            (1)


#if HAVE_UNSIGNED_PROP	
typedef  unsigned int       PROP_TYPE;
#else
typedef  int       			PROP_TYPE;
#endif 

#define MAX_PROP            (INT_MAX - 1)

#define kDamp               (0.85f)
#define kDampFixPoint       108//(0.85 << 7)  // * 128


#define BLK_SIZE            (512*1024)
#define VERTEX_MAX          (BLK_SIZE)
#define ENDFLAG             0xffffffff


#define BURSTBUFFERSIZE         (128)
#define LOG_BURSTBUFFERSIZE     (7)

#define ALIGN_SIZE              (BURSTBUFFERSIZE * 16)


#define PE_NUM              16
#define EDGE_NUM            8
#define LOG2_PE_NUM         4
#define LOG2_EDGE_NUM       (LOG2_PE_NUM - 1)

#define HASH_MASK           (PE_NUM - 1)


//#define SW_DEBUG

#define BRAM_INIT				(0)

#define CAHCE_FETCH_DEBUG   	(0)


#define VERTEX_ACTIVE_BIT_MASK 		(0x80000000)


#if HAVE_VERTEX_ACTIVE_BIT

#define IS_ACTIVE_VERTEX(a)			((((((a) & VERTEX_ACTIVE_BIT_MASK) == VERTEX_ACTIVE_BIT_MASK))) ? 1 : 0)

#else

#define IS_ACTIVE_VERTEX(a)			(1)

#endif

#endif /* __CONFIG_H__ */
