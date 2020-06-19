#include "host_graph_sw.h"

#include "host_graph_scheduler.h"


static int stepCounter = 0;

int normalInitHanlder(void *arg)
{
    stepCounter = 0;
    return 0;
}


int normalSubPartitionArrangementHandler(partitionDescriptor * partition)
{
    if (stepCounter % 2 == 0)
    {
        for (int k = 0; k < SUB_PARTITION_NUM; k++)
        {
            partition->finalOrder[k] = k;
        }
    }
    else
    {
        for (int k = 0; k < SUB_PARTITION_NUM; k++)
        {
            partition->finalOrder[k] = SUB_PARTITION_NUM - k - 1;
        }
    }

    return 0;
}

int normalSchedulerPartitionArrangement(partitionDescriptor * partitions, int * table, int size)
{
    for (int i = 0; i < size; i++)
    {
        table[i] = i;
    }

    return 0;
}

static graphStaticScheduler dut = {
    .init                   = normalInitHanlder,
    .subPartionScheduler    = normalSubPartitionArrangementHandler,
    .partitionScheduler     = normalSchedulerPartitionArrangement,
};



int schedulerRegister(void)
{
    return registerScheduler(&dut);
}
