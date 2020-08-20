#ifndef __GLOBAL_CONFIG_H__
#define __GLOBAL_CONFIG_H__

#include <climits>
#include <math.h>


#if HAVE_UNSIGNED_PROP
typedef  unsigned int       prop_t;
#else
typedef  int                prop_t;
#endif


#define BLK_SIZE                                (512*1024)
#define MAX_VERTICES_IN_ONE_PARTITION           (BLK_SIZE)
#define ENDFLAG                                 0xffffffff


#define LOG_BURSTBUFFERSIZE     (7)
#define BURSTBUFFERSIZE         (1<<7)


#define ALIGN_SIZE              (BURSTBUFFERSIZE * 16)


//#define SW_DEBUG

#define CAHCE_FETCH_DEBUG       (0)

#define HAVE_GS             (1)
#define HAVE_FPGA           (1)
#define HAVE_SW             (0)





#endif /* __GLOBAL_CONFIG_H__ */
