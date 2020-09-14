
#include "host_graph_sw.h"


#define HW_EMU_DEBUG        (0)
#define HW_EMU_DEBUG_SIZE   (16384 * 4)

gatherScatterDescriptor localGsKernel[] = {
    {
        .name = "readEdgesCU1",
    },
    {
        .name = "readEdgesCU2",
    },
    {
        .name = "readEdgesCU3",
    },
    {
        .name = "readEdgesCU4",
    },
};

applyDescriptor localApplyKernel[] =
{
    {
        .name = "vertexApply",
    },
};

gatherScatterDescriptor * getGatherScatter(int kernelID)
{
    return &localGsKernel[kernelID];
}

applyDescriptor * getApply(void)
{
    return &localApplyKernel[DEFAULT_KERNEL_ID];
}

void kernelInit(graphAccelerator * acc)
{
    cl_int status;
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        getGatherScatter(i)->kernel = clCreateKernel(acc->program, getGatherScatter(i)->name, &status);
        checkStatus("Failed clCreateKernel gs.");
        acc->gsKernel[i] =  getGatherScatter(i);
    }

#if HAVE_APPLY

    getApply()->kernel = clCreateKernel(acc->program, getApply()->name, &status);
    checkStatus("Failed clCreateKernel apply.");
    acc->applyKernel = getApply();

#endif

    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        acc->gsOps[i] = clCreateCommandQueue(acc->context, acc->device,   CL_QUEUE_PROFILING_ENABLE, &status);
        checkStatus("Failed clCreateCommandQueue of gsOps.");
    }
    acc->applyOps = clCreateCommandQueue(acc->context, acc->device,  CL_QUEUE_PROFILING_ENABLE, &status);
    checkStatus("Failed clCreateCommandQueue of applyOps.");
}

void setGsKernel(int partId, int superStep, graphInfo *info)
{
    int currentPropId = superStep % 2;

    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        gatherScatterDescriptor * gsHandler = getGatherScatter(i);
        subPartitionDescriptor * partition = getSubPartition(partId * SUB_PARTITION_NUM + i);
        int argvi = 0;
        int edgeEnd     = partition->listEnd;
        int sinkStart   = 0;
        int sinkEnd     = MAX_VERTICES_IN_ONE_PARTITION;

#if HW_EMU_DEBUG
        edgeEnd         = HW_EMU_DEBUG_SIZE;
#endif

        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partition->edgeHead.id));
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(gsHandler->prop[currentPropId].id));
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partition->edgeTail.id));

        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partition->tmpProp.id));
        he_set_dirty(partition->tmpProp.id);

#if HAVE_EDGE_PROP
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partition->edgeProp.id));
#endif
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(int),    &edgeEnd);
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(int),    &sinkStart);
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(int),    &sinkEnd);
    }
}

#if  CUSTOMIZE_APPLY == 0


void setApplyKernel(int partId, int superStep, graphInfo *info)
{
#if HAVE_APPLY
    int currentPropId = superStep % 2;
    int updatePropId  = (superStep + 1) % 2;

    applyDescriptor * applyHandler = getApply();
    int argvi = 0;
    unsigned int argReg = dataPrepareGetArg(info);
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
                       get_cl_mem_pointer(getSubPartition(partId * SUB_PARTITION_NUM + i)->tmpProp.id)
                      );
    }
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem),
                       get_cl_mem_pointer(getGatherScatter(i)->prop[updatePropId].id)
                      );
        he_set_dirty(getGatherScatter(i)->prop[updatePropId].id);
    }

#if HAVE_APPLY_OUTDEG
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(MEM_ID_OUT_DEG));
#endif
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(MEM_ID_RESULT_REG));
    he_set_dirty(MEM_ID_RESULT_REG);

    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(int),    &sink_end);
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(int),    &offset);
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(int),    &argReg);
#endif
}

#endif
