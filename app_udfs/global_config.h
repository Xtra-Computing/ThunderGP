#ifndef __GLOBAL_CONFIG_H__
#define __GLOBAL_CONFIG_H__

#include <climits>
#include <math.h>


#if HAVE_UNSIGNED_PROP
typedef  unsigned int       prop_t;
#else
typedef  int                prop_t;
#endif

#ifdef TARGET_PARTITION_SIZE
#undef PARTITION_SIZE
#define PARTITION_SIZE  (TARGET_PARTITION_SIZE)
#endif
#define MAX_VERTICES_IN_ONE_PARTITION           (PARTITION_SIZE)

#define ENDFLAG                                 0xffffffff

#define LOG_BURSTBUFFERSIZE     (7)
#define BURSTBUFFERSIZE         (1<<7)
#define ALIGN_SIZE              (BURSTBUFFERSIZE * 16)

//#define SW_DEBUG

#define CAHCE_FETCH_DEBUG       (0)





#endif /* __GLOBAL_CONFIG_H__ */
