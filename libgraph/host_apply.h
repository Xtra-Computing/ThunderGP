#ifndef __HOST_APPLY_H__
#define __HOST_APPLY_H__


void setApplyKernel(
    cl_kernel &kernel_apply,
    int partId,
    int vertexNum
);



void partitionApplyCModel(
    cl_context              &context,
    cl_device_id            &device,
    int                     partId,
    unsigned int            baseScore
);

#endif /* __HOST_APPLY_H__ */
