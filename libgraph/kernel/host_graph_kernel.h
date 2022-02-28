#ifndef __HOST_GRAPH_SW_KERNEL_H__
#define __HOST_GRAPH_SW_KERNEL_H__

void kernelInit(graphAccelerator * acc);

void setGsKernel(int partId, int superStep, graphInfo *info);

void setApplyKernel(int partId, int superStep, graphInfo *info);


#endif /* __HOST_GRAPH_SW_KERNEL_H__ */
