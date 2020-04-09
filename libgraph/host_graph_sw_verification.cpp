
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <stdlib.h>
#include <malloc.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <vector>
#include <math.h>
#include <ctime>

#include <sys/time.h>

#include "host_graph_sw.h"


//#define SW_DEBUG

#define DEBUG_DUMP_VERTEX_SIZE          (1024)
//#define PROBE_VERTEX                    (160018)
#define SRC_VERTEX_CHECK                (1)
#define SRC_VERTEX_CHECK_UNIT           (16)


#define DATA_DUMP                       (0) // i < 50


extern partitionDescriptor partitions[MAX_PARTITIONS_NUM];
extern gs_cu_t localGsKernel[SUB_PARTITION_NUM];


void singleThreadSWProcessing(
    CSR                     *csr,
    const int               &blkNum,
    const int               &vertexNum,
    const int               &source
)
{

    int *rpa = (int*)get_host_mem_pointer(MEM_ID_RPA);
    int *cia =    (int*)get_host_mem_pointer(MEM_ID_CIA);
    int *vertexProp =    (int*)get_host_mem_pointer(MEM_ID_VERTEX_PROP);
    int *outDeg =    (int*)get_host_mem_pointer(MEM_ID_OUT_DEG);
    int *vertexScore =    (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE);
    int *tmpVertexProp =    (int*)get_host_mem_pointer(MEM_ID_TMP_VERTEX_PROP);
    printf("vertexNum for singleThreadSWProcessing %d\n", vertexNum);
    int base_score = float2int((1.0f - kDamp) / vertexNum);
    printf("base_score original %.*f \n", 10, (1.0f - kDamp) / vertexNum);
    printf("base_score int %d \n", base_score);
    printf("base_score after int %.*f\n", 10, int2float(base_score));
    int itNum = 0;
    while (itNum < MAX_ITER) {
        //std::cout << "Processing with partition, iteration: " << itNum[0] << std::endl;
        //#pragma omp parallel for
        for (int u = 0; u < vertexNum; u++) {
            int start = rpa[u];
            int num = rpa[u + 1] - rpa[u];
            for (int j = 0; j < num; j++) {
                tmpVertexProp[cia[start + j]] += vertexProp[u] / (csr->rpao[u + 1] - csr->rpao[u]);
            }
        }
        int error = 0;
        //#pragma omp parallel for reduction(+:error)
        for (int i = 0; i < vertexNum; i++) {
            PROP_TYPE tProp = tmpVertexProp[i];
            PROP_TYPE old_score = vertexProp[i];
            vertexProp[i] = base_score + kDamp * tProp;
            error += fabs(vertexProp[i] - old_score);
            tmpVertexProp[i] = 0;
            if (outDeg[i] > 0)
            {
                vertexScore[i] = vertexProp[i] / outDeg[i];
            }
            else
            {
                vertexScore[i] = 0;
            }
        }
        printf(" %2d    %lf\n", itNum, int2float(error));
        itNum++;
    }
}


void swVerifyCmodel(
    fpga_task_info_t task_info
)
{
    PROP_TYPE *edgesTuples   = (int*)get_host_mem_pointer(MEM_ID_EDGE_TUPLES);
    int *edgeScoreMap        = (int*)get_host_mem_pointer(MEM_ID_EDGE_SCORE_MAP);
    int *vertexScore         = (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE_CACHED);
    int *tmpVertexPropVerify = (int*)get_host_mem_pointer(MEM_ID_TMP_VERTEX_VERIFY);
    int *edgeProp            = (int*)get_host_mem_pointer(MEM_ID_EDGE_PROP);

    for (int i = 0; i < task_info.edge_end; i++)
    {
        tmpVertexPropVerify[edgesTuples[i]] += vertexScore[edgeScoreMap[i]] ; //* edgeProp[i];
#ifdef PROBE_VERTEX
        if (edgesTuples[i] == PROBE_VERTEX)
        {
            printf("probe (%d): %d %d %d %d \n", edgesTuples[i], i, edgeScoreMap[i], vertexScore[edgeScoreMap[i]], tmpVertexPropVerify[edgesTuples[i]]);
        }
#endif

    }

    int sw_error = 0;

    int *tmpVertexProp =  (int*)get_host_mem_pointer(MEM_ID_TMP_VERTEX_PROP);
    int *vertexProp    =  (int*)get_host_mem_pointer(MEM_ID_VERTEX_PROP);

    //int *outDeg        =  (int*)get_host_mem_pointer(MEM_ID_OUT_DEG);
    //int *vertexScore   =  (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE);
    for (int i = 0; i < task_info.sink_end; i++) {
        PROP_TYPE tProp = tmpVertexProp[i];
        PROP_TYPE old_score = vertexProp[i];
        int tmpProp = task_info.base_score + ((kDampFixPoint * tProp) >> 7);
        if (i < DEBUG_DUMP_VERTEX_SIZE)
        {
        }
        sw_error += abs(tmpProp - old_score);
        //if (outDeg[i] > 0) vertexScore[i] = vertexProp[i] / outDeg[i];
    }
    printf("-----------------------------------------------------------------\n");
    printf("[RES] sw error  %lf\n", int2float(sw_error));
    printf("[RES] hw error  %lf\n", int2float(((int *)get_host_mem_pointer(MEM_ID_ERROR))[0]));

    int error_count = 0;
    int total_count = 0;
    for (int i = 0; i < task_info.sink_end; i ++) {
        if (tmpVertexPropVerify[i] != 0)
        {
            total_count ++;
        }
        if (tmpVertexPropVerify[i] != tmpVertexProp[i])
        {
            error_count++;


#ifndef SW_DEBUG
            if (error_count <= 100)
            {
                printf("error tmp  %d 0x%08x 0x%08x  diff 0x%08x !!!!\n", i,
                       tmpVertexProp[i],
                       tmpVertexPropVerify[i],
                       tmpVertexPropVerify[i] - tmpVertexProp[i]);
            }
#endif
        }
        else
        {
#ifdef SW_DEBUG
            if (total_count < 1024)
            {
                printf("dump tmp  %d 0x%08x 0x%08x\n", i, tmpVertexProp[i], tmpVertexPropVerify[i]);
            }
#endif
        }
    }
    printf("[RES] error_count %d  in size %d/ %d\n", error_count, total_count, task_info.sink_end);
}


void partitionGatherScatterCModel(
    cl_context              &context,
    cl_device_id            &device,
    int                     cuIndex,
    partitionDescriptor     *partitions
)
{
    PROP_TYPE *edgesTuples   = (int*)get_host_mem_pointer(partitions->edge.id);
    int *edgeScoreMap        = (int*)get_host_mem_pointer(partitions->edgeMap.id);
    int *vertexScore         = (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE_CACHED);
    int *tmpVertexPropVerify = (int*)get_host_mem_pointer(MEM_ID_TMP_VERTEX_VERIFY);
    int *edgeProp            = (int*)get_host_mem_pointer(partitions->edgeProp.id);
    clear_host_mem(MEM_ID_TMP_VERTEX_VERIFY);
    DEBUG_PRINTF("partition cmodel verify:\n");

    for (unsigned int i = 0; i < partitions->listEnd; i++)
    {
        tmpVertexPropVerify[edgesTuples[i]] += vertexScore[edgeScoreMap[i]] ; //* edgeProp[i];
#ifdef PROBE_VERTEX
        if (edgesTuples[i] == PROBE_VERTEX)
        {
            DEBUG_PRINTF("probe (%d): %d %d %d %d \n", edgesTuples[i], i, edgeScoreMap[i], vertexScore[edgeScoreMap[i]], tmpVertexPropVerify[edgesTuples[i]]);
        }
#endif
    }
    transfer_data_from_pl(context, device, partitions->tmpProp.id);


    int *tmpVertexProp =  (int*)get_host_mem_pointer(partitions->tmpProp.id);

    int error_count = 0;
    int total_count = 0;
    for (unsigned int i = partitions->dstVertexStart; i < partitions->dstVertexEnd; i ++) {
        if (tmpVertexPropVerify[i] != 0)
        {
            total_count ++;
        }
        if (tmpVertexPropVerify[i] != tmpVertexProp[i - partitions->dstVertexStart])
        {
            error_count++;


#ifndef SW_DEBUG
            if (error_count <= 100)
            {
                DEBUG_PRINTF("error tmp  %d 0x%08x 0x%08x  diff 0x%08x !!!!\n", i,
                             tmpVertexProp[i],
                             tmpVertexPropVerify[i],
                             tmpVertexPropVerify[i] - tmpVertexProp[i]);
            }
#endif
        }
        else
        {
#ifdef SW_DEBUG
            if (total_count < 1024)
            {
                DEBUG_PRINTF("dump tmp  %d 0x%08x 0x%08x\n", i, tmpVertexProp[i], tmpVertexPropVerify[i]);
            }
#endif
        }
    }
    DEBUG_PRINTF("[RES] error_count %d  in size %d/ %d\n", error_count, total_count,
                 partitions->dstVertexEnd - partitions->dstVertexStart + 1 );
}



void partitionApplyCModel(
    cl_context              &context,
    cl_device_id            &device,
    int                     partId,
    unsigned int            baseScore
)
{
    for (int i  = 0; i < SUB_PARTITION_NUM; i++)
    {
        transfer_data_from_pl(context, device, partitions[partId * SUB_PARTITION_NUM + i].tmpProp.id);

    }
    int* pCuData[SUB_PARTITION_NUM];
    int* updateVerify = (int*)get_host_mem_pointer(MEM_ID_VERTEX_PROP_VERIFY);
    int* outDeg       = (int*)get_host_mem_pointer(MEM_ID_OUT_DEG);
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        pCuData[i] = (int*)get_host_mem_pointer(partitions[partId * SUB_PARTITION_NUM + i].tmpProp.id);
    }
    int offset = partitions[partId * SUB_PARTITION_NUM].dstVertexStart;
    for (int i = 0; i < VERTEX_MAX; i++)
    {
        int mergeData = 0;

        for (int j = 0 ; j < SUB_PARTITION_NUM; j++)
        {
            int* pLocal = pCuData[j];
            mergeData += pLocal[i];
        }
        int tProp = mergeData;
        int newScore = baseScore  + ((kDampFixPoint * tProp) >> 7);
        if (outDeg[i + offset] != 0)
        {
            updateVerify[i] = newScore / outDeg[i + offset];
        }
        else
        {
            updateVerify[i] = 0;
        }
    }
    int error_count = 0;
    transfer_data_from_pl(context, device, localGsKernel[0].propUpdate.id);
    int* hwUpdate = (int *)get_host_mem_pointer(localGsKernel[0].propUpdate.id);
    for (int i = 0; i < VERTEX_MAX; i++)
    {
        if (updateVerify[i] !=  hwUpdate[i + offset])
        {
            error_count ++;
            if (error_count < 50)
            {

                DEBUG_PRINTF("apply error tmp  %d 0x%08x 0x%08x  diff 0x%08x !!!!\n", i,
                             updateVerify[i],
                             hwUpdate[i + offset],
                             updateVerify[i] - hwUpdate[i + offset]);
            }
        }
        if (DATA_DUMP)
        {
            DEBUG_PRINTF("%d 0x%08x 0x%08x  diff 0x%08x \n", i,
                         updateVerify[i],
                         hwUpdate[i + offset],
                         updateVerify[i] - hwUpdate[i + offset]);
        }
    }
    DEBUG_PRINTF("[RES] apply error_count %d \n", error_count);
}


