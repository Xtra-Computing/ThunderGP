#ifndef __HOST_GRAPH_DATA_STRUCTURE_H__
#define __HOST_GRAPH_DATA_STRUCTURE_H__

#include "graph.h"

#define MAX_PARTITIONS_NUM      (128)


typedef struct
{
    double fpgaExeTime;
    double effic;
    double compress;
    double degree;
} profileLog;

typedef struct
{
    unsigned int  listStart;
    unsigned int  listEnd;
    unsigned int  dstVertexStart;
    unsigned int  dstVertexEnd;
    unsigned int  srcVertexStart;
    unsigned int  srcVertexEnd;
    float         scatterCacheRatio;
    float         compressRatio;
    he_mem_t      edgeTail;
    he_mem_t      edgeHead;
    he_mem_t      edgeProp;
    he_mem_t      tmpProp;
    unsigned int  mapedTotalIndex;
    unsigned int  cuIndex;
    profileLog log;
} subPartitionDescriptor;

typedef struct
{
    subPartitionDescriptor  *sub[SUB_PARTITION_NUM];
    int                     finalOrder[SUB_PARTITION_NUM];
    unsigned int            totalEdge;
    unsigned int            subPartitionSize;
    cl_event                syncEvent[SUB_PARTITION_NUM];
    cl_event                applyEvent;
    double                  applyExeTime;
} partitionDescriptor;

typedef struct
{
    const char* name;
    int partition_mem_attr;
    int prop_id;
    int output_id;
    cl_kernel kernel;
    he_mem_t  prop[2];
    he_mem_t  tmpProp;
} gatherScatterDescriptor;

typedef struct
{
    const char* name;
    cl_kernel kernel;
} applyDescriptor;

typedef struct
{
    CSR* csr;

    subPartitionDescriptor subPartitions[MAX_PARTITIONS_NUM * SUB_PARTITION_NUM];

    partitionDescriptor partitions[MAX_PARTITIONS_NUM];

    gatherScatterDescriptor * gsKernel[SUB_PARTITION_NUM];

    applyDescriptor * applyKernel;

    cl_command_queue gsOps[SUB_PARTITION_NUM];

    cl_command_queue applyOps;

    cl_program program;

    cl_platform_id platform;

    cl_device_id device;

    cl_context context;

} graphAccelerator;


subPartitionDescriptor * getSubPartition(int partID);
partitionDescriptor * getPartition(int partID);

gatherScatterDescriptor * getGatherScatter(int kernelID);
applyDescriptor * getApply(void);

graphAccelerator * getAccelerator(void);


// inline int getCuIDbyInterface(int order)
// {
//     return he_get_interface_id(order);
// }


#endif /* __HOST_GRAPH_DATA_STRUCTURE_H__ */
