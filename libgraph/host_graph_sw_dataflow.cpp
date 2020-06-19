
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

int acceleratorSuperStep(int step, graphInfo *info)
{
    graphAccelerator * acc = getAccelerator();
    int blkNum = info->blkNum;
    for (int i = 0; i < blkNum + 1; i ++)
    {
        if (i < blkNum)
        {
            partitionDescriptor * partition = getPartition(i);
            setGsKernel(getArrangedPartitionID(i), step, info);
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

            setApplyKernel(getArrangedPartitionID(i - 1), step, info);
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


int accelratorProfile (int step, graphInfo *info, double exeTime)
{
    graphAccelerator * acc = getAccelerator();
    int blkNum = info->blkNum;
    for (int i = 0; i < blkNum; i ++)
    {
#if HAVE_APPLY
        partitionDescriptor * partition = getPartition(i);
        unsigned long applyTime = xcl_get_event_duration(partition->applyEvent);
#else
        unsigned long applyTime = 0;
#endif
        for (int j = 0; j < SUB_PARTITION_NUM; j ++)
        {
            unsigned long fpgaTime = xcl_get_event_duration(partition->syncEvent[j]);
            partition->sub[j]->log.fpgaExeTime = fpgaTime;
        }
        partition->applyExeTime = applyTime;

    }

    /* verification */
    if (step == 0)
    {
        for (int i = 0; i < blkNum; i ++)
        {
            partitionDescriptor * partition = getPartition(i);
            for (int j = 0; j < SUB_PARTITION_NUM; j ++)
            {
                partitionGatherScatterCModel(acc->context, acc->device, j, partition->sub[j]);
            }
#if HAVE_APPLY
            partitionApplyCModel(acc->context, acc->device, getArrangedPartitionID(i), dataPrepareGetArg(info));
#endif
        }

    }
    /* log */
    if (step > 1)
    {
        double fpga_runtime_total = 0;
        double end2end_runtime_total = 0;
        for (int i = 0;  i < blkNum; i++)
        {
            double max_fpga_exe = 0;
            partitionDescriptor * partition = getPartition(i);
            for (int j = 0; j < SUB_PARTITION_NUM; j ++)
            {
                if (partition->sub[j]->log.fpgaExeTime > max_fpga_exe)
                {
                    max_fpga_exe = partition->sub[j]->log.fpgaExeTime ;
                }
                DEBUG_PRINTF("[INFO] part_gs %d cu%d exe: %f \n", i, j, partition->sub[j]->log.fpgaExeTime / 1000000.0);
            }
            end2end_runtime_total = exeTime;
            fpga_runtime_total    += max_fpga_exe / 1000000.0;
            DEBUG_PRINTF("[INFO] part_apply %d exe: %f \n", i, partition->applyExeTime / 1000000.0);
#if 0
            DEBUG_PRINTF("[INFO] partedge %f fpga gs: %f ms, apply: %f ms %d, effic %lf  v/e %lf compress %lf \n",
                         exeTime,
                         max_fpga_exe / 1000000.0,
                         
                         partition->subPartitionSize,
                         (((float)partition->subPartitionSize) / subPartitions[i * SUB_PARTITION_NUM].mapedTotalIndex),
                         ((subPartitions[i * SUB_PARTITION_NUM].dstVertexEnd - subPartitions[i * SUB_PARTITION_NUM].dstVertexStart)
                          / ((float)(subPartitions[i * SUB_PARTITION_NUM].listEnd - subPartitions[i * SUB_PARTITION_NUM].listStart))),
                         subPartitions[i * SUB_PARTITION_NUM].compressRatio);
#endif

        }
        DEBUG_PRINTF("[INFO] summary e2e %lf fpga %lf\n",
                     end2end_runtime_total,
                     fpga_runtime_total)
    }
    return 0;
}

int acceleratorDeinit(void)
{
    graphAccelerator * acc = getAccelerator();
    /* TODO free other resource */

    if (acc->context)            clReleaseContext(acc->context);

    return 0;
}
