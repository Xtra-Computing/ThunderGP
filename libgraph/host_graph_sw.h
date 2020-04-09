#ifndef __HOST_GRAPH_SW_H__
#define __HOST_GRAPH_SW_H__

#include <stdint.h>

#include "common.h"

#include "graph.h"
#include "safequeue.h"

#include "he_mem.h"
#include "he_mem_id.h"



#define MAX_PARTITIONS_NUM      (128)


#define checkStatus(str) {                          \
    if (status != 0 || status != CL_SUCCESS) {      \
        printf("Error code: %d\n", status);         \
        printf("Error: %s\n", str);                 \
        freeResources();                            \
        exit(-1);                                   \
    }                                               \
}

typedef struct
{
    int edge_start;
    int edge_end;
    int sink_start;
    int sink_end;
    int base_score;
} fpga_task_info_t;

typedef struct
{
    const char* name;
    int partition_mem_attr;
    int prop_id;
    int output_id;
    cl_kernel kernel;
    cl_event *event[MAX_PARTITIONS_NUM];
    he_mem_t  prop;
    he_mem_t  propUpdate;
    he_mem_t  tmpProp;
} gs_cu_t;

typedef struct
{
    double end2end_exe;
    double fpga_exe;
    double apply_exe;
    double effic;
    double compress;
    double degree;
} profile_log_t;

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
    he_mem_t      edge;
    he_mem_t      edgeMap;
    he_mem_t      edgeProp;
    he_mem_t      tmpProp;
    unsigned int  mapedTotalIndex;
    unsigned int  cuIndex;
    profile_log_t log;
} partitionDescriptor;

int float2int(float a);
float int2float(int a);
double getCurrentTimestamp(void);

void freeResources(void);

Graph* createGraph(const std::string &gName, const std::string &mode);

void mem_init(CSR * csr, cl_context &context);

void processInit(
    const int   &vertexNum,
    const int   &edgeNum,
    const int   &source
);


void processMemInit(cl_context &context);

void swVerify(
    CSR                     *csr,
    const int               &vertexNum,
    int                     endoffset,
    int                     *rpa,
    int                     *cia,
    int                     *vertexProp,
    int                     *vertexScore,
    int                     *outDeg,
    int                     *tmpVertexProp
);

void swVerifyCmodel( fpga_task_info_t task_info);

void partitionGatherScatterCModel(
    cl_context              &context,
    cl_device_id            &device,
    int                     cuIndex,
    partitionDescriptor     *partitions
);

void partitionApplyCModel(
    cl_context              &context,
    cl_device_id            &device,
    int                     partId,
    unsigned int            baseScore
);

void singleThreadSWProcessing(
    CSR                     *csr,
    const int               &blkNum,
    const int               &vertexNum,
    const int               &source
);

void partitionFunction(
    CSR                     *csr,
    int                     &blkNum,
    cl_context              &context,
    partitionDescriptor     *partitions
);

#endif /* __HOST_GRAPH_SW_H__ */
