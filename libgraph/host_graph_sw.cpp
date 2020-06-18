
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

#include "he_mem_config.h"


#define EDEG_MEMORY_SIZE        ((edgeNum + (ALIGN_SIZE * 4) * blkNum) * 1)
#define VERTEX_MEMORY_SIZE      (((vertexNum - 1)/VERTEX_MAX + 1) * VERTEX_MAX)


subPartitionDescriptor subPartitions[MAX_PARTITIONS_NUM * SUB_PARTITION_NUM];

int partIdTable[MAX_PARTITIONS_NUM];

gatherScatterDescriptor localGsKernel[] = {
    {
        .name = "readEdgesCU1",
    },
    {
        .name = "readEdgesCU2",
    },
    {
        .name = "readEdgesCU3",
    },
    {
        .name = "readEdgesCU4",
    },
};

const int sizeAttrLut[] = {ATTR_PL_DDR3, ATTR_PL_DDR2, ATTR_PL_DDR1, ATTR_PL_DDR0};

const int cuAttrLut[] = {ATTR_PL_DDR3, ATTR_PL_DDR2, ATTR_PL_DDR1, ATTR_PL_DDR0};


#define PARTITION_DDR       (sizeAttrLut[cuIndex])
#define CU_DDR              (cuAttrLut[cuIndex])


void base_mem_init(cl_context &context)
{
    for (unsigned int i = 0; i < ARRAY_SIZE(local_mem); i++)
    {
        he_mem_init(context, &local_mem[i]);
    }
}

subPartitionDescriptor * getPartition(int partID)
{
    return &subPartitions[partID];
}


gatherScatterDescriptor * getGatherScatter(int kernelID)
{
    return &localGsKernel[kernelID];
}


void setGsKernel(int partId)
{
#if HAVE_GS
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        gatherScatterDescriptor * gsHandler = getGatherScatter(i); 
        subPartitionDescriptor * partition = getPartition(partId * SUB_PARTITION_NUM + i);
        int argvi = 0;
        int edgeEnd     = partition->listEnd;
        int sinkStart   = 0;
        int sinkEnd     = VERTEX_MAX;

#if HW_EMU_DEBUG
        edgeEnd         = HW_EMU_DEBUG_SIZE;
#endif
        //DEBUG_PRINTF("gs task in cu [%d] info:\n", i);
        //DEBUG_PRINTF("\tedge  %d %d \n", 0, edgeEnd);
        //DEBUG_PRINTF("\tsink  %d %d \n", sinkStart, sinkEnd);
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partition->edgeMap.id));
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(gsHandler->prop.id));
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partition->edge.id));

        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partition->tmpProp.id));

#if HAVE_EDGE_PROP
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partition->edgeProp.id));
#endif
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(int),    &edgeEnd);
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(int),    &sinkStart);
        clSetKernelArg(gsHandler->kernel, argvi++, sizeof(int),    &sinkEnd);
    }
#endif
}

#if  CUSTOMIZE_APPLY == 0

void setApplyKernel(cl_kernel &kernel_apply, int partId, int vertexNum)
{
#if HAVE_APPLY
    int argvi = 0;
    int base_score = float2int((1.0f - kDamp) / vertexNum);
    subPartitionDescriptor *p_partition = getPartition(partId * SUB_PARTITION_NUM);

    volatile unsigned int partitionVertexNum = ((p_partition->dstVertexEnd - p_partition->dstVertexStart)
            / (ALIGN_SIZE ) + 1) * (ALIGN_SIZE );
    int sink_end = partitionVertexNum;
    int offset = p_partition->dstVertexStart;

    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(2)->prop.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getPartition(partId * SUB_PARTITION_NUM + 2)->tmpProp.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getPartition(partId * SUB_PARTITION_NUM + 1)->tmpProp.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getPartition(partId * SUB_PARTITION_NUM + 0)->tmpProp.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getPartition(partId * SUB_PARTITION_NUM + 3)->tmpProp.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(2)->propUpdate.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(1)->propUpdate.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(0)->propUpdate.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(3)->propUpdate.id));
#if HAVE_APPLY_OUTDEG
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(MEM_ID_OUT_DEG));
#endif    
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(MEM_ID_ERROR));

    clSetKernelArg(kernel_apply, argvi++, sizeof(int),    &sink_end);
    clSetKernelArg(kernel_apply, argvi++, sizeof(int),    &offset);
    clSetKernelArg(kernel_apply, argvi++, sizeof(int),    &base_score);
#endif
}

#endif




void gs_mem_init(cl_context &context, gatherScatterDescriptor *gsItem, int cuIndex, void *data)
{
    gsItem->prop.id =  MEM_ID_GS_BASE + cuIndex * MEM_ID_GS_OFFSET;
    gsItem->prop.name = "cu prop";
    gsItem->prop.attr = CU_DDR;
    gsItem->prop.unit_size = sizeof(int);
    gsItem->prop.size_attr = SIZE_IN_VERTEX;
    he_mem_init(context, &gsItem->prop);

    gsItem->tmpProp.id =  MEM_ID_GS_BASE + cuIndex * MEM_ID_GS_OFFSET + 1;
    gsItem->tmpProp.name = "cu output tmpProp";
    gsItem->tmpProp.attr = CU_DDR;
    gsItem->tmpProp.unit_size = sizeof(int);
    gsItem->tmpProp.size_attr = SIZE_IN_VERTEX;
    he_mem_init(context, &gsItem->tmpProp);

    gsItem->propUpdate.id =  MEM_ID_GS_BASE + cuIndex * MEM_ID_GS_OFFSET + 2;
    gsItem->propUpdate.name = "cu propUpdate";
    gsItem->propUpdate.attr = CU_DDR;
    gsItem->propUpdate.unit_size = sizeof(int);
    gsItem->propUpdate.size_attr = SIZE_IN_VERTEX;
    he_mem_init(context, &gsItem->propUpdate);

    memcpy(gsItem->prop.data, data, gsItem->prop.size);
    memcpy(gsItem->propUpdate.data, data, gsItem->propUpdate.size);
}


void processMemInit(cl_context &context)
{
    int *vertexProp =       (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE_CACHED);
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        gs_mem_init(context, &localGsKernel[i], i, vertexProp);
    }
}

void partition_mem_init(cl_context &context, int blkIndex, int size, int cuIndex)
{
    int i = blkIndex;
    subPartitionDescriptor *partitionItem  = &subPartitions[i];
    {
        partitionItem->cuIndex = cuIndex;
        partitionItem->edge.id = MEM_ID_PARTITION_BASE + i * MEM_ID_PARTITION_OFFSET;
        partitionItem->edge.name = "partition edge";
        partitionItem->edge.attr = PARTITION_DDR;
        partitionItem->edge.unit_size = size * sizeof(int);
        partitionItem->edge.size_attr = SIZE_USER_DEFINE;
        he_mem_init(context, &partitionItem->edge);

        partitionItem->edgeMap.id = MEM_ID_PARTITION_BASE + i * MEM_ID_PARTITION_OFFSET + 1;
        partitionItem->edgeMap.name = "partition edgeMap";
        partitionItem->edgeMap.attr = PARTITION_DDR;
        partitionItem->edgeMap.unit_size = size * sizeof(int);
        partitionItem->edgeMap.size_attr = SIZE_USER_DEFINE;
        he_mem_init(context, &partitionItem->edgeMap);

        partitionItem->edgeProp.id = MEM_ID_PARTITION_BASE + i * MEM_ID_PARTITION_OFFSET + 2;
        partitionItem->edgeProp.name = "partition edgeProp";
#if 1
        partitionItem->edgeProp.attr = PARTITION_DDR;
#else
        partitionItem->edgeProp.attr = ATTR_HOST_ONLY;
#endif
        partitionItem->edgeProp.unit_size = size * sizeof(int);
        partitionItem->edgeProp.size_attr = SIZE_USER_DEFINE;
        he_mem_init(context, &partitionItem->edgeProp);

        partitionItem->tmpProp.id = MEM_ID_PARTITION_BASE + i * MEM_ID_PARTITION_OFFSET + 3;
        partitionItem->tmpProp.name = "partition tmpProp";
        partitionItem->tmpProp.attr = PARTITION_DDR;
        partitionItem->tmpProp.unit_size = VERTEX_MAX * sizeof(int);
        partitionItem->tmpProp.size_attr = SIZE_USER_DEFINE;
        he_mem_init(context, &partitionItem->tmpProp);
    }
}
void mem_init(CSR * csr, cl_context &context)
{
    int vertexNum = csr->vertexNum;
    int edgeNum   = csr->edgeNum;
    int blkNum    = (vertexNum + BLK_SIZE - 1) / BLK_SIZE;

    printf("blkNum %d  %d \n", blkNum,  EDEG_MEMORY_SIZE);

    register_size_attribute(SIZE_IN_EDGE     , EDEG_MEMORY_SIZE  );
    register_size_attribute(SIZE_IN_VERTEX   , VERTEX_MEMORY_SIZE);
    register_size_attribute(SIZE_USER_DEFINE , 1                 );

    base_mem_init(context);

    int *rpa        = (int*)get_host_mem_pointer(MEM_ID_RPA);
    int *cia        = (int*)get_host_mem_pointer(MEM_ID_CIA);
    int *outDeg     = (int*)get_host_mem_pointer(MEM_ID_OUT_DEG);

    for (int i = 0; i < vertexNum; i++) {
        if (i < csr->vertexNum) { // 'vertexNum' may be aligned.
            rpa[i] = csr->rpao[i];
            outDeg[i] = csr->rpao[i + 1] - csr->rpao[i];
        }
        else {
            rpa[i] = 0;
            outDeg[i] = 0;
        }
    }
    rpa[vertexNum] = csr->rpao[vertexNum];
    for (int i = 0; i < edgeNum; i++) {
        cia[i] = csr->ciao[i];
    }
}


void processInit(
    const int   &vertexNum,
    const int   &edgeNum,
    const int   &source
)
{
    int *vertexProp =       (int*)get_host_mem_pointer(MEM_ID_VERTEX_PROP);
    int *outDeg =           (int*)get_host_mem_pointer(MEM_ID_OUT_DEG);
    int *vertexScore =      (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE);
    unsigned int *activeVertexNum = (unsigned int *)get_host_mem_pointer(MEM_ID_ACTIVE_VERTEX_NUM);
    unsigned int *activeVertices  = (unsigned int *)get_host_mem_pointer(MEM_ID_ACTIVE_VERTEX);


    float init_score_float = 1.0f / vertexNum;
    int init_score_int = float2int(init_score_float);
    printf("init_score %d\n", init_score_int);

    /*
    for (int i = 0; i < vertexNum; i++) {
        vertexProp[i] = init_score_int;
        if (outDeg[i] > 0)
        {
            vertexScore[i] =  vertexProp[i] / outDeg[i];
            if (i < 128)
            {
                //printf("%d %d %d %d \n", i, vertexScore[i], vertexProp[i], outDeg[i] );
            }
        }
        else
        {
            vertexScore[i]  = 0;
        }
    }
    */
    for (int i = 0; i < (vertexNum / (ALIGN_SIZE ) + 1) * (ALIGN_SIZE ); i++) {
        vertexScore[i]    = MAX_PROP;
    }
    //vertexScore[source] = 0x80000001;
    activeVertexNum[0] = 1;
    activeVertices[0] = source;

    printf("init_score original %f \n", init_score_float);
    printf("init_score original to int %d \n", init_score_int);
    printf("init_score after int %f\n", int2float(init_score_int));
}


double cuPerformance[SUB_PARTITION_NUM];


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




void partitionFunction(
    CSR                     *csr,
    int                     &blkNum,
    cl_context              &context,
    subPartitionDescriptor     *subPartitions
)
{
    memset(partIdTable, 0, sizeof(int) * MAX_PARTITIONS_NUM);
    int *rpa = (int*)get_host_mem_pointer(MEM_ID_RPA);
    int *cia = (int*)get_host_mem_pointer(MEM_ID_CIA);
    int *vertexScoreCached    = (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE_CACHED);
    int *vertexScore          = (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE);
    unsigned int *vertexMap   = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_MAP);
    unsigned int *vertexRemap = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_REMAP);


    unsigned int mapedSourceIndex = 0;
    int vertexNum = csr->vertexNum;

    for (int u = 0; u < vertexNum; u++) {
        int num = rpa[u + 1] - rpa[u];
        vertexMap[u] = mapedSourceIndex;
        if (num != 0)
        {
#if CAHCE_FETCH_DEBUG
            vertexScoreCached[mapedSourceIndex] =  mapedSourceIndex;
#else
            vertexScoreCached[mapedSourceIndex] =  vertexScore[u];
#endif
            vertexRemap[mapedSourceIndex] = u;
            mapedSourceIndex ++;
        }
    }
    
    int startIndx = getStartIndex();
    vertexScoreCached[startIndx] = 0x80000001;

    processMemInit(context);

    blkNum =  (mapedSourceIndex + BLK_SIZE - 1) / BLK_SIZE;
    printf("blkNum:%d  vertexNum: %d \n", blkNum, vertexNum);


    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        cuPerformance[i] = 0;
    }

    for (int i = 0; i < blkNum; i ++) {
        /****************************************************************************************************************/
#if 0
        unsigned int *bitmapOri =  (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_BIT_ORI);
        //for (int delta_index = 1; delta_index < 16; delta_index ++)
        int delta_index = (BURSTBUFFERSIZE / 2);
        {
#define  DELTA (16 * delta_index)
            for (int u = 0; u < vertexNum; u++) {
                bitmapOri[u / DELTA] = 0;
            }
            mapedSourceIndex = 0;

            for (int u = 0; u < vertexNum; u++) {
                int start = rpa[u];
                int num = rpa[u + 1] - rpa[u];
                for (int j = 0; j < num; j++) {
                    int cia_idx = start + j; //printf("cia_idx %d\n",cia_idx );
                    int vertex_idx = vertexMap[cia[cia_idx]];
                    if ((vertex_idx >= i * VERTEX_MAX) && (vertex_idx < (i + 1) * VERTEX_MAX)) {
                        bitmapOri[mapedSourceIndex / DELTA] = 1;
                    }
                }
                if (num != 0)
                {
                    mapedSourceIndex ++;
                }
            }
            int hit_counter = 0;
            for (int u = 0; u < vertexNum / DELTA ; u++) {
                if (bitmapOri[u] == 1)
                {
                    hit_counter++;
                }
            }

            subPartitions[i].scatterCacheRatio =  ((float)hit_counter) / ((mapedSourceIndex / DELTA + 1 ));
            DEBUG_PRINTF("[TRS] scatter cache ratio: %lf in %d @blk %d \n",
                         subPartitions[i].scatterCacheRatio,
                         DELTA, i);
            /* calculate scatter efficient */
        }
#endif
        /****************************************************************************************************************/
        uint cur_edge_num = 0;
        int *edgesTuples  = (int*)get_host_mem_pointer(MEM_ID_EDGE_TUPLES);
        int *edgeScoreMap = (int*)get_host_mem_pointer(MEM_ID_EDGE_SCORE_MAP);
        int *edgeProp     = (int*)get_host_mem_pointer(MEM_ID_EDGE_PROP);
        mapedSourceIndex = 0;

        for (int u = 0; u < vertexNum; u++) {
            int start = rpa[u];
            int num = rpa[u + 1] - rpa[u];
            for (int j = 0; j < num; j++) {
                //tmpVertexProp[cia[start + j]] += vertexScore[u];//vertexProp[u] / (csr->rpao[u+1] - csr->rpao[u]);
                int cia_idx = start + j; //printf("cia_idx %d\n",cia_idx );
                int vertex_idx = vertexMap[cia[cia_idx]];
                if ((vertex_idx >= i * VERTEX_MAX) && (vertex_idx < (i + 1) * VERTEX_MAX)) {
                    edgesTuples[cur_edge_num] = vertex_idx;
                    edgeScoreMap[cur_edge_num] = mapedSourceIndex;
                    //edgeProp[cur_edge_num] = cur_edge_num & 0xFFFFFF; //SpMV test
                    edgeProp[cur_edge_num] = 1;
                    cur_edge_num ++;
                }
            }
            if (num != 0)
            {
                mapedSourceIndex ++;
            }
        }


        DEBUG_PRINTF("\nunpad edge_tuple_range %d\n", cur_edge_num);


        int unpad_edge_num = cur_edge_num;
        //align at 4k

        for (int k = 0; k < (ALIGN_SIZE - (unpad_edge_num % ALIGN_SIZE)); k ++) {
            edgesTuples[cur_edge_num] = (ENDFLAG - 1);
            edgeScoreMap[cur_edge_num] = edgeScoreMap[cur_edge_num - 1];
            edgeProp[cur_edge_num] = 0;
            cur_edge_num ++; //printf("%d edge_tuple_range %d\n", k, cur_edge_num);
        }
        
        double currentEst[SUB_PARTITION_NUM];
        double currentEstLut[SUB_PARTITION_NUM];
        int    reOrderIndexArray[SUB_PARTITION_NUM];
        int    finalOrder[SUB_PARTITION_NUM];
        for (int subIndex = 0 ; subIndex < SUB_PARTITION_NUM ; subIndex ++ )
        {
            int  partId = i * SUB_PARTITION_NUM + subIndex;
            subPartitions[partId].compressRatio = (double (mapedSourceIndex)) / vertexNum;
            DEBUG_PRINTF("ratio %d / %d is %lf \n", mapedSourceIndex, vertexNum, subPartitions[partId].compressRatio);
            subPartitions[partId].dstVertexStart = VERTEX_MAX * (i);
            subPartitions[partId].dstVertexEnd   = ((unsigned int)(VERTEX_MAX * (i + 1)) > mapedSourceIndex) ? mapedSourceIndex : VERTEX_MAX * (i + 1) ;
            volatile int subPartitionSize = ((cur_edge_num / SUB_PARTITION_NUM) / ALIGN_SIZE) * ALIGN_SIZE;
            int reOrderIndex = subIndex;
            unsigned int bound = subPartitionSize * (reOrderIndex + 1);

            int totalEdge = (bound > cur_edge_num) ? (cur_edge_num - (subPartitionSize * reOrderIndex)) : (subPartitionSize);
            int totalVertex = edgeScoreMap[subPartitionSize * (reOrderIndex + 1) - 1] -  edgeScoreMap[subPartitionSize * reOrderIndex];
            currentEst[subIndex] = performanceEstimator(totalVertex, totalEdge);
            currentEstLut[subIndex] = currentEst[subIndex];
            reOrderIndexArray[subIndex] = subIndex;
            finalOrder[subIndex] = subIndex;
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
            finalOrder[maxIndex] = reOrderIndexArray[SUB_PARTITION_NUM - k - 1];
        }
        for (int k = 0; k < SUB_PARTITION_NUM; k++)
        {
            cuPerformance[k] += currentEstLut[finalOrder[k]];
            DEBUG_PRINTF("[EST]: finalOrder %d total exe: %lfms\n", finalOrder[k], cuPerformance[k] / 1000000.0);
        }


        for (int subIndex = 0 ; subIndex < SUB_PARTITION_NUM ; subIndex ++ )
        {
            int  partId = i * SUB_PARTITION_NUM + subIndex;
            volatile int subPartitionSize = ((cur_edge_num / SUB_PARTITION_NUM) / ALIGN_SIZE) * ALIGN_SIZE;
            int reOrderIndex = finalOrder[subIndex];

            unsigned int bound = subPartitionSize * (reOrderIndex + 1);

            subPartitions[partId].listStart =  0;
            subPartitions[partId].listEnd = (bound > cur_edge_num) ? (cur_edge_num - (subPartitionSize * reOrderIndex)) : (subPartitionSize);
            subPartitions[partId].mapedTotalIndex = mapedSourceIndex;

            DEBUG_PRINTF("[SIZE] %d cur_edge_num sub %d\n", cur_edge_num, subPartitions[partId].listEnd);
            partition_mem_init(context, partId, subPartitions[partId].listEnd, subIndex); // subIndex --> cuIndex
            memcpy(subPartitions[partId].edge.data     , &edgesTuples[subPartitionSize * reOrderIndex]  , subPartitions[partId].listEnd * sizeof(int));
            memcpy(subPartitions[partId].edgeMap.data  , &edgeScoreMap[subPartitionSize * reOrderIndex] , subPartitions[partId].listEnd * sizeof(int));
            memcpy(subPartitions[partId].edgeProp.data , &edgeProp[subPartitionSize * reOrderIndex]     , subPartitions[partId].listEnd * sizeof(int));
        }
    }

#define CACHE_UNIT_SIZE (4096 * 2)
    for (int i = 0; i < blkNum; i++)
    {
        int *edgeScoreMap = (int*)subPartitions[i].edgeMap.data;
        DEBUG_PRINTF("\n----------------------------------------------------------------------------------\n");
        DEBUG_PRINTF("[PART] subPartitions %d info :\n", i);
        DEBUG_PRINTF("[PART] \t edgelist from %d to %d\n"   , subPartitions[i].listStart      , subPartitions[i].listEnd     );
        DEBUG_PRINTF("[PART] \t dst. vertex from %d to %d\n", subPartitions[i].dstVertexStart , subPartitions[i].dstVertexEnd);
        DEBUG_PRINTF("[PART] \t src. vertex from %d to %d\n", subPartitions[i].srcVertexStart , subPartitions[i].srcVertexEnd);
        DEBUG_PRINTF("[PART] \t dump: %d - %d\n", (edgeScoreMap[subPartitions[i].listStart]), (edgeScoreMap[subPartitions[i].listEnd - 1]));
        DEBUG_PRINTF("[PART] scatter cache ratio %lf \n", subPartitions[i].scatterCacheRatio);
        DEBUG_PRINTF("[PART] v/e %lf \n", (subPartitions[i].dstVertexEnd - subPartitions[i].dstVertexStart)
                     / ((float)(subPartitions[i].listEnd - subPartitions[i].listStart)));

        DEBUG_PRINTF("[PART] v: %d e: %d \n", (subPartitions[i].dstVertexEnd - subPartitions[i].dstVertexStart),
                     (subPartitions[i].listEnd - subPartitions[i].listStart));

        DEBUG_PRINTF("[PART] est. efficient %lf\n", ((float)(subPartitions[i].listEnd - subPartitions[i].listStart)) / mapedSourceIndex);

        DEBUG_PRINTF("[PART] compressRatio %lf \n\n", subPartitions[i].compressRatio);


#if SRC_VERTEX_CHECK
        int lastMapIndex = edgeScoreMap[subPartitions[i].listStart];
        int inner_counter = 0;
        DEBUG_PRINTF("SRC_VERTEX_CHECK @%d\n", SRC_VERTEX_CHECK_UNIT);
        int max_diff  = 0;
        int diff_count[3] = {0};
        for (unsigned int j = subPartitions[i].listStart;  j < subPartitions[i].listEnd; j += SRC_VERTEX_CHECK_UNIT)
        {
            inner_counter ++;
            int diff = edgeScoreMap[j] - lastMapIndex;
            if (diff > max_diff)
            {
                max_diff = diff;
            }
            if (diff > 2000)
            {
                DEBUG_PRINTF("hit: edge index over cache perfetch [%d] %d %d %d --- %d \n",
                             inner_counter, j, lastMapIndex, edgeScoreMap[j], diff);
                diff_count[0] ++;
            } else if (diff > 1000)
            {
                diff_count[1] ++;
            }
            else if (diff > 100)
            {
                diff_count[2] ++;
            }

            //if ((diff) >  SRC_VERTEX_CHECK_UNIT )
            //{
            //DEBUG_PRINTF("warning: edge index over cache perfetch [%d] %d %d %d --- %d \n",
            //             inner_counter, j, lastMapIndex, edgeScoreMap[j], diff);
            //}
            lastMapIndex = edgeScoreMap[j];
        }
        DEBUG_PRINTF(" MAX DIFF[%d] %d %d %d\n",
                     max_diff, diff_count[0], diff_count[1], diff_count[2]);

#endif

    }
    for (int i = 0; i < blkNum; i++)
    {
        partIdTable[i] = i;
    }
#if 1
    for (int i = 0; i < blkNum; i++)
    {
        for (int j = 0; j < blkNum - i - 1; j++)
        {
            if (subPartitions[partIdTable[j] * SUB_PARTITION_NUM].listEnd < subPartitions[partIdTable[j + 1] * SUB_PARTITION_NUM].listEnd)
            {
                int tmpId = partIdTable[j];
                partIdTable[j] = partIdTable[j + 1];
                partIdTable[j + 1]  = tmpId;
            }
        }
    }
#endif
    for (int i = 0; i < blkNum; i++)
    {
        DEBUG_PRINTF("[SCHE] %d with %d @ %d \n", partIdTable[i], subPartitions[partIdTable[i] * SUB_PARTITION_NUM].listEnd, i);
    }
    int paddingVertexNum  = ((vertexNum  - 1) / (ALIGN_SIZE * 4) + 1 ) * (ALIGN_SIZE * 4);
    int *tmpVertexProp =  (int*)get_host_mem_pointer(MEM_ID_TMP_VERTEX_PROP);
    int *vertexProp    =  (int*)get_host_mem_pointer(MEM_ID_VERTEX_PROP);
    int base_score = float2int((1.0f - kDamp) / vertexNum);
    for (int i = vertexNum; i < paddingVertexNum; i++)
    {
        tmpVertexProp[i] = 0;
        vertexProp[i] = base_score;
    }

}





