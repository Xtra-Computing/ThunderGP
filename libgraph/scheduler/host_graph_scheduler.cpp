#include "host_graph_sw.h"

#include "host_graph_scheduler.h"

static graphStaticScheduler scheduler;

static int partIdTable[MAX_PARTITIONS_NUM];

int registerScheduler(graphStaticScheduler * pItem)
{
    if (pItem == NULL)
    {
        return -1;
    }
    scheduler.init                 = pItem->init;
    scheduler.subPartionScheduler  = pItem->subPartionScheduler;
    scheduler.partitionScheduler   = pItem->partitionScheduler;
    return 0;
}

int getArrangedPartitionID(int step)
{
    return partIdTable[step];
}

int schedulerInit(void *arg)
{
    memset(partIdTable, 0, sizeof(int) * MAX_PARTITIONS_NUM);
    if (scheduler.init == NULL)
        return 0;
    return scheduler.init(arg);
}

int schedulerSubPartitionArrangement(int partIndex)
{
    partitionDescriptor * partition = getPartition(partIndex);
    if (scheduler.subPartionScheduler == NULL)
    {
        for (int i = 0; i < SUB_PARTITION_NUM; i++)
        {
            partition->finalOrder[i] = i;
        }
        return 0;
    }
    return scheduler.subPartionScheduler(partIndex);
}


int schedulerPartitionArrangement(int size)
{
    if (scheduler.partitionScheduler == NULL)
    {
        for (int i = 0; i < size; i++)
        {
            partIdTable[i] = i;
        }
        return 0;
    }
    return scheduler.partitionScheduler(partIdTable, size);
}