
scalar_t global;

void setApplyKernel(int partId, int superStep, graphInfo *info)
{
    int currentPropId = superStep % 2;
    int updatePropId  = (superStep + 1) % 2;

    applyDescriptor * applyHandler = getApply();
    int argvi = 0;
    subPartitionDescriptor *p_partition = getSubPartition(partId * SUB_PARTITION_NUM);

    volatile unsigned int partitionVertexNum = ((p_partition->dstVertexEnd - p_partition->dstVertexStart)
            / (ALIGN_SIZE ) + 1) * (ALIGN_SIZE );
    int sink_end = partitionVertexNum;
    int offset = p_partition->dstVertexStart;


    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem),
                   get_cl_mem_pointer(getGatherScatter(getCuIDbyInterface(DEFAULT_KERNEL_ID))->prop[currentPropId].id));

    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem),
                       get_cl_mem_pointer(getSubPartition(partId * SUB_PARTITION_NUM + (i))->tmpProp.id)
                      );
    }
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem),
                       get_cl_mem_pointer(getGatherScatter((i))->prop[updatePropId].id)
                      );
        he_set_dirty(getGatherScatter((i))->prop[updatePropId].id);
    }

#pragma THUNDERGP USER_APPLY_CL_KERNEL
// mark for auto generation

    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(int),    &sink_end);
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(int),    &offset);

}