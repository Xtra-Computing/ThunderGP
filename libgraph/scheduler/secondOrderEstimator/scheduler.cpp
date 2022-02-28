#include "host_graph_sw.h"

#include "host_graph_scheduler.h"



static double cuPerformance[SUB_PARTITION_NUM];



double performanceEstimator(double vertex, double edge)
{
    const double p00 =   3.096e+05;
    const double p10 =      0.6971;
    const double p01 =      0.2648;
    const double p11 =  -1.076e-08;
    const double p02 =   4.187e-08;
    double x = edge;
    double y = vertex;
    double time = p00 + p10 * x + p01 * y + p11 * x * y + p02 * y * y;
    return time;
}


int soeInitHanlder(void *arg)
{
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        cuPerformance[i] = 0;
    }
    return 0;
}


int soeSubPartitionArrangementHandler(int partIndex)
{
    partitionDescriptor * partition =getPartition(partIndex);
    double currentEst[SUB_PARTITION_NUM];
    double currentEstLut[SUB_PARTITION_NUM];
    int    reOrderIndexArray[SUB_PARTITION_NUM];

    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        unsigned int subPartitionSize = partition->subPartitionSize;
        unsigned int bound = subPartitionSize * (i + 1);

        int subTotalEdge = (bound > partition->totalEdge) ? (partition->totalEdge - (subPartitionSize * i)) : (subPartitionSize);
        int subTotalVertex = partition->sub[i]->srcVertexEnd - partition->sub[i]->srcVertexStart;
        currentEst[i] = performanceEstimator(subTotalVertex, subTotalEdge);
        currentEstLut[i] = currentEst[i];
        reOrderIndexArray[i] = i;
        partition->finalOrder[i] = i;
    }

    for (int k = 0; k < SUB_PARTITION_NUM; k++)
    {
        for (int j = 0; j < SUB_PARTITION_NUM - k - 1; j++)
        {
            if (currentEst[j] < currentEst[j + 1])
            {
                int tmpId = reOrderIndexArray[j];
                double tmpEst = currentEst[j];

                reOrderIndexArray[j] = reOrderIndexArray[j + 1];
                reOrderIndexArray[j + 1]  = tmpId;

                currentEst[j] = currentEst[j + 1];
                currentEst[j + 1] = tmpEst;
            }
        }
    }
    for (int k = 0; k < SUB_PARTITION_NUM; k++)
    {
        DEBUG_PRINTF("[EST]: %d is expected to exe in %lfms\n", reOrderIndexArray[k], currentEst[k] / 1000000.0);
    }
    int tmpMap[SUB_PARTITION_NUM];
    for (int k = 0; k < SUB_PARTITION_NUM; k++)
    {
        tmpMap[k] = 0;
    }

    for (int k = 0; k < SUB_PARTITION_NUM; k++)
    {
        double maxPerf = -1;
        int    maxIndex = SUB_PARTITION_NUM;
        for (int j = 0; j < SUB_PARTITION_NUM; j++)
        {
            if (tmpMap[j] == 0)
            {
                if (maxPerf < cuPerformance[j])
                {
                    maxPerf = cuPerformance[j];
                    maxIndex = j;
                }
            }
        }
        tmpMap[maxIndex] = 1;
        partition->finalOrder[maxIndex] = reOrderIndexArray[SUB_PARTITION_NUM - k - 1];
    }
    for (int k = 0; k < SUB_PARTITION_NUM; k++)
    {
        cuPerformance[k] += currentEstLut[partition->finalOrder[k]];
        DEBUG_PRINTF("[EST]: finalOrder %d total exe: %lfms\n", partition->finalOrder[k], cuPerformance[k] / 1000000.0);
    }

    return 0;
}

int soeSchedulerPartitionArrangement(int * table, int size)
{
    for (int i = 0; i < size; i++)
    {
        table[i] = i;
    }
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size - i - 1; j++)
        {
            if (getPartition(table[j])->totalEdge < getPartition(table[j + 1])->totalEdge)
            {
                int tmpId = table[j];
                table[j] = table[j + 1];
                table[j + 1]  = tmpId;
            }
        }
    }
    return 0;
}

static graphStaticScheduler dut = {
    .init                   = soeInitHanlder,
    .subPartionScheduler    = soeSubPartitionArrangementHandler,
    .partitionScheduler     = soeSchedulerPartitionArrangement,
};



int schedulerRegister(void)
{
    return registerScheduler(&dut);
}
