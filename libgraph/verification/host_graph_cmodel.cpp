#include "host_graph_sw.h"

#include "global_config.h"
#include "fpga_application.h"

#include "host_graph_verification.h"



int acceleratorCModelSuperStep(int superStep, graphInfo *info)
{
    int currentPropId = superStep % 2;
    int resultPropId = (superStep + 1) % 2;
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