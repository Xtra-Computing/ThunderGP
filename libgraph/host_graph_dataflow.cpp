
#include "host_graph_sw.h"
#include "host_graph_scheduler.h"

graphAccelerator thunderGraph;

graphAccelerator * getAccelerator(void)
{
    return &thunderGraph;
}

subPartitionDescriptor * getSubPartition(int partID)
{
    return &thunderGraph.subPartitions[partID];
}

partitionDescriptor * getPartition(int partID)
{
    return &thunderGraph.partitions[partID];
}



int acceleratorInit(const char * name, char *file_name)
{
    cl_int  status;
    cl_uint numPlatforms;
    cl_uint numDevices;
    graphAccelerator * acc = getAccelerator();

    status = clGetPlatformIDs(1, &(acc->platform), &numPlatforms);
    checkStatus("Failed clGetPlatformIDs.");
    DEBUG_PRINTF("Found %d platforms!\n", numPlatforms);

    status = clGetDeviceIDs(acc->platform, CL_DEVICE_TYPE_ALL, 1, &(acc->device), &numDevices);
    checkStatus("Failed clGetDeviceIDs.");
    DEBUG_PRINTF("Found %d devices!\n", numDevices);

    acc->context = clCreateContext(0, 1, &(acc->device), NULL, NULL, &status);
    checkStatus("Failed clCreateContext.");

    xcl_world world = xcl_world_single();
    acc->program = xcl_import_binary(world, name, file_name);

    kernelInit(acc);

    return 0;
}

int acceleratorSuperStep(int superStep, graphInfo *info)
{
    graphAccelerator * acc = getAccelerator();
    int blkNum = info->blkNum;
    for (int i = 0; i < blkNum + 1; i ++)
    {
        if (i < blkNum)
        {
            partitionDescriptor * partition = getPartition(i);
            setGsKernel(getArrangedPartitionID(i), superStep, info);
            for (int j = 0; j < SUB_PARTITION_NUM; j ++)
            {
                clEnqueueTask(acc->gsOps[j], getGatherScatter(j)->kernel, 0, NULL,
                              &partition->syncEvent[j]);
            }
        }
#if HAVE_APPLY
        if (i > 0)
        {
            partitionDescriptor * lastPartition;
            lastPartition = getPartition(i - 1);

            setApplyKernel(getArrangedPartitionID(i - 1), superStep, info);
            clEnqueueTask(acc->applyOps, getApply()->kernel, SUB_PARTITION_NUM,
                          lastPartition->syncEvent,
                          &lastPartition->applyEvent);
        }
#endif
    }

    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        clFinish(acc->gsOps[i]);
    }
    clFinish(acc->applyOps);
    return 0;
}




int acceleratorDeinit(void)
{
    graphAccelerator * acc = getAccelerator();
    /* TODO free other resource */

    if (acc->context)            clReleaseContext(acc->context);

    return 0;
}


void* acceleratorQueryRegister(void)
{
    graphAccelerator * acc = getAccelerator();
    transfer_data_from_pl(acc->context, acc->device,MEM_ID_RESULT_REG);
    return get_host_mem_pointer(MEM_ID_RESULT_REG);
}

prop_t* acceleratorQueryProperty(int step)
{
    graphAccelerator * acc = getAccelerator();
    transfer_data_from_pl(acc->context, acc->device, getGatherScatter(0)->prop[step].id);
    prop_t * propValue = (prop_t *)get_host_mem_pointer(getGatherScatter(0)->prop[step].id);

    return propValue;
}

