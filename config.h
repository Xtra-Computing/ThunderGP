#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <climits>
#include <math.h>


#if HAVE_UNSIGNED_PROP	
typedef  unsigned int       prop_t;
#else
typedef  int       			prop_t;
#endif 


#define BLK_SIZE            (512*1024)
#define VERTEX_MAX          (BLK_SIZE)
#define ENDFLAG             0xffffffff


#define BURSTBUFFERSIZE         (128)
#define LOG_BURSTBUFFERSIZE     (7)

#define ALIGN_SIZE              (BURSTBUFFERSIZE * 16)


//#define SW_DEBUG

#define CAHCE_FETCH_DEBUG   	(0)

#define HAVE_GS             (1)
#define HAVE_FPGA           (1)
#define HAVE_SW             (0)





#endif /* __CONFIG_H__ */
