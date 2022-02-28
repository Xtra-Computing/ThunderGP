#ifndef __HOST_GRAPH_SCHEDULER__
#define __HOST_GRAPH_SCHEDULER__


typedef int (* schedulerInitHanlder)(void *arg);

typedef int (* subPartitionArrangementHandler)(int partIndex);

typedef int (* partitionArrangementHandler)(int * table, int size);


typedef struct{
    schedulerInitHanlder              init;
    subPartitionArrangementHandler    subPartionScheduler;
    partitionArrangementHandler       partitionScheduler;
} graphStaticScheduler;

int schedulerRegister(void);

int registerScheduler(graphStaticScheduler * pItem);


/* internal phase */

int schedulerInit(void *arg);

int schedulerSubPartitionArrangement(int partIndex);

int schedulerPartitionArrangement(int size);

int getArrangedPartitionID(int step);


#endif /* __HOST_GRAPH_SCHEDULER__ */
