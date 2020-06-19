
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
#if HAVE_GS
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        getGatherScatter(i)->kernel = clCreateKernel(acc->program, getGatherScatter(i)->name, &status);
        checkStatus("Failed clCreateKernel gs.");
        acc->gsKernel[i] =  getGatherScatter(i);
    }
#endif

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
    int currentPropId = superStep%2;

#if HAVE_GS
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        gatherScatterDescriptor * gsHandler = getGatherScatter(i);
        subPartitionDescriptor * partition = getSubPartition(partId * SUB_PARTITION_NUM + i);
        int argvi = 0;
        int edgeEnd     = partition->listEnd;
        int sinkStart   = 0;
        int sinkEnd     = VERTEX_MAX;

#if HW_EMU_DEBUG
        edgeEnd         = HW_EMU_DEBUG_SIZE;
#endif
        //DEBUG_PRINTF("gs task in cu [%d] info:\n", i);
        //DEBUG_PRINTF("\tedge  %d %d \n", 0, edgeEnd);
        //DEBUG_PRINTF("\tsink  %d %d \n", sinkStart, sinkEnd);
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partition->edgeMap.id));
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(gsHandler->prop[currentPropId].id));
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partition->edge.id));

        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partition->tmpProp.id));

#if HAVE_EDGE_PROP
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partition->edgeProp.id));
#endif
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(int),    &edgeEnd);
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(int),    &sinkStart);
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(int),    &sinkEnd);
    }
#endif
}

#if  CUSTOMIZE_APPLY == 0

int applyGlobalMemoryIndex[] =
{
    2, 1, 0, 3
};

void setApplyKernel(int partId, int superStep, graphInfo *info)
{
    int currentPropId = superStep % 2;
    int updatePropId  = (superStep + 1) % 2;
#if HAVE_APPLY
    applyDescriptor * applyHandler = getApply();
    int argvi = 0;
    unsigned int argReg = dataPrepareGetArg(info);
    subPartitionDescriptor *p_partition = getSubPartition(partId * SUB_PARTITION_NUM);

    volatile unsigned int partitionVertexNum = ((p_partition->dstVertexEnd - p_partition->dstVertexStart)
            / (ALIGN_SIZE ) + 1) * (ALIGN_SIZE );
    int sink_end = partitionVertexNum;
    int offset = p_partition->dstVertexStart;


    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(2)->prop[currentPropId].id));

    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getSubPartition(partId * SUB_PARTITION_NUM + 2)->tmpProp.id));
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getSubPartition(partId * SUB_PARTITION_NUM + 1)->tmpProp.id));
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getSubPartition(partId * SUB_PARTITION_NUM + 0)->tmpProp.id));
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getSubPartition(partId * SUB_PARTITION_NUM + 3)->tmpProp.id));
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(2)->prop[updatePropId].id));
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(1)->prop[updatePropId].id));
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(0)->prop[updatePropId].id));
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(3)->prop[updatePropId].id));
#if HAVE_APPLY_OUTDEG
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(MEM_ID_OUT_DEG));
#endif
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(MEM_ID_ERROR));

    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(int),    &sink_end);
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(int),    &offset);
    clSetKernelArg(applyHandler->kernel, argvi++, sizeof(int),    &argReg);
#endif
}

#endif