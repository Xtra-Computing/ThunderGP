#ifndef __FPGA_APPLICATION_H__
#define __FPGA_APPLICATION_H__


//#define PROP_COMPUTE_STAGE1(srcProp, edgeProp)    ((srcProp) * (edgeProp))

//#define PROP_COMPUTE_STAGE2(ori, update)  		((ori) + (update))

//#define PROP_COMPUTE_STAGE3(ori, update)  		((ori) + (update))


/* source vertex property process */
#define PROP_COMPUTE_STAGE0(srcProp)    		 	((srcProp) + 1)

/* source vertex property & edge property */
#define PROP_COMPUTE_STAGE1(srcProp, edgeProp)    	((srcProp) + (edgeProp))

/* destination property update in RAW solver */
#define PROP_COMPUTE_STAGE2(ori, update)    		((((ori)& (~VERTEX_ACTIVE_BIT_MASK)) > ((update) & (~VERTEX_ACTIVE_BIT_MASK)))?(update):(ori))

/* destination property update dst buffer update */
#define PROP_COMPUTE_STAGE3(ori,update)    			(((((ori)& (~VERTEX_ACTIVE_BIT_MASK)) > ((update) & (~VERTEX_ACTIVE_BIT_MASK))) || (ori == 0x0))?(update):(ori))


/* destination property merge */
#define PROP_COMPUTE_STAGE4(ori,update)    			((((((ori)& (~VERTEX_ACTIVE_BIT_MASK)) > ((update) & (~VERTEX_ACTIVE_BIT_MASK))) && (update != 0)) || (ori == 0x0))?(update):(ori))


#endif /* __FPGA_APPLICATION_H__ */
