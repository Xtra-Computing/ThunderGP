#ifndef __FPGA_APPLICATION_H__
#define __FPGA_APPLICATION_H__



#define VERTEX_ACTIVE_BIT_MASK 		(0x80000000)


#if HAVE_VERTEX_ACTIVE_BIT

#define IS_ACTIVE_VERTEX(a)			((((((a) & VERTEX_ACTIVE_BIT_MASK) == VERTEX_ACTIVE_BIT_MASK))) ? 1 : 0)

#else

#define IS_ACTIVE_VERTEX(a)			(1)

#endif


#include "l2.h"

/* source vertex property process */
#define PROP_COMPUTE_STAGE0(srcProp)                preprocessProperty(srcProp)

/* source vertex property & edge property */
#define PROP_COMPUTE_STAGE1(srcProp, edgeProp)      updateCalculation(srcProp,edgeProp)

/* destination property update in RAW solver */
#define PROP_COMPUTE_STAGE2(ori, update)            updateMergeInRAWSolver(ori ,update)

/* destination property update dst buffer update */
#define PROP_COMPUTE_STAGE3(ori,update)             updateDestination(ori, update)

/* destination property merge */
#define PROP_COMPUTE_STAGE4(ori,update)             applyMerge(ori, update)


#endif /* __FPGA_APPLICATION_H__ */
