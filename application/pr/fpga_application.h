#ifndef __FPGA_APPLICATION_H__
#define __FPGA_APPLICATION_H__


//#define PROP_COMPUTE_STAGE1(srcProp, edgeProp)    ((srcProp) * (edgeProp))

//#define PROP_COMPUTE_STAGE2(ori, update)  		((ori) + (update))

//#define PROP_COMPUTE_STAGE3(ori, update)  		((ori) + (update))


/* source vertex property process */
#define PROP_COMPUTE_STAGE0(srcProp)    		 	(srcProp)

/* source vertex property & edge property */
#define PROP_COMPUTE_STAGE1(srcProp, edgeProp)    	((srcProp) * (edgeProp))

/* destination property update in RAW solver */
#define PROP_COMPUTE_STAGE2(ori, update)    		((ori) + (update))

/* destination property update dst buffer update */
#define PROP_COMPUTE_STAGE3(ori,update)    			((ori) + (update))


/* destination property merge */
#define PROP_COMPUTE_STAGE4(ori,update)    			((ori) + (update))


#endif /* __FPGA_APPLICATION_H__ */
