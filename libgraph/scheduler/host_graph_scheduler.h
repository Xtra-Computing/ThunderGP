#ifndef __HOST_GRAPH_SCHEDULER__
#define __HOST_GRAPH_SCHEDULER__


typedef int (* schedulerInitHanlder)(void *arg);

typedef int (* subPartitionArrangementHandler)(partitionDescriptor * partition);

typedef int (* partitionArrangementHandler)(partitionDescriptor * partitions, int * table, int size);



typedef struct{
    schedulerInitHanlder              init;
    subPartitionArrangementHandler    subPartionScheduler;
    partitionArrangementHandler       partitionScheduler;
} graphStaticScheduler;


int registerScheduler(graphStaticScheduler * pItem);



/* internal phase */

int schedulerInit(void *arg);

int schedulerSubPartitionArrangement(partitionDescriptor * partition);

int schedulerPartitionArrangement(partitionDescriptor * partitions, int size);

int getArrangedPartitionID(int step);


#endif /* __HOST_GRAPH_SCHEDULER__ */
