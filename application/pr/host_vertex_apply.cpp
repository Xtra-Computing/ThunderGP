#include "host_graph_sw.h"

#include "fpga_application.h"

#include "host_graph_sw_verification.h"


void setApplyKernel(cl_kernel &kernel_apply, int partId, int vertexNum)
{
#if HAVE_APPLY
    int argvi = 0;
    int base_score = float2int((1.0f - kDamp) / vertexNum);
    partitionDescriptor *p_partition = getPartition(partId * SUB_PARTITION_NUM);

    volatile unsigned int partitionVertexNum = ((p_partition->dstVertexEnd - p_partition->dstVertexStart)
            / (ALIGN_SIZE ) + 1) * (ALIGN_SIZE );
    int sink_end = partitionVertexNum;
    int offset = p_partition->dstVertexStart;

    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(2)->prop.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getPartition(partId * SUB_PARTITION_NUM + 2)->tmpProp.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getPartition(partId * SUB_PARTITION_NUM + 1)->tmpProp.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getPartition(partId * SUB_PARTITION_NUM + 0)->tmpProp.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getPartition(partId * SUB_PARTITION_NUM + 3)->tmpProp.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(2)->propUpdate.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(1)->propUpdate.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(0)->propUpdate.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(3)->propUpdate.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(MEM_ID_OUT_DEG));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(MEM_ID_ERROR));

    clSetKernelArg(kernel_apply, argvi++, sizeof(int),    &sink_end);
    clSetKernelArg(kernel_apply, argvi++, sizeof(int),    &offset);
    clSetKernelArg(kernel_apply, argvi++, sizeof(int),    &base_score);
#endif
}


prop_t  applyVerfication(prop_t tProp,prop_t source, unsigned int outDeg, void * arg)
{
    prop_t updateVerify = 0;
    unsigned int baseScore = *((unsigned int *)arg);
    int newScore = baseScore  + ((kDampFixPoint * tProp) >> 7);
    if (outDeg != 0)
    {
        updateVerify = newScore / outDeg;
    }
    return updateVerify;
}



