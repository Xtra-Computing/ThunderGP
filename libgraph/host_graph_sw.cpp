
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


#include "host_graph_scheduler.h"


void process_mem_init(cl_context &context);
void partition_mem_init(cl_context &context, int blkIndex, int size, int cuIndex);


subPartitionDescriptor subPartitions[MAX_PARTITIONS_NUM * SUB_PARTITION_NUM];

partitionDescriptor partitions[MAX_PARTITIONS_NUM];


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


subPartitionDescriptor * getSubPartition(int partID)
{
    return &subPartitions[partID];
}

partitionDescriptor * getPartition(int partID)
{
    return &partitions[partID];
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
        subPartitionDescriptor * partition = getSubPartition(partId * SUB_PARTITION_NUM + i);
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
    subPartitionDescriptor *p_partition = getSubPartition(partId * SUB_PARTITION_NUM);

    volatile unsigned int partitionVertexNum = ((p_partition->dstVertexEnd - p_partition->dstVertexStart)
            / (ALIGN_SIZE ) + 1) * (ALIGN_SIZE );
    int sink_end = partitionVertexNum;
    int offset = p_partition->dstVertexStart;

    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getGatherScatter(2)->prop.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getSubPartition(partId * SUB_PARTITION_NUM + 2)->tmpProp.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getSubPartition(partId * SUB_PARTITION_NUM + 1)->tmpProp.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getSubPartition(partId * SUB_PARTITION_NUM + 0)->tmpProp.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(getSubPartition(partId * SUB_PARTITION_NUM + 3)->tmpProp.id));
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


extern int schedulerRegister(void);

void processInit(
    const int   &vertexNum,
    const int   &edgeNum,
    const int   &source
)
{
    schedulerRegister();
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






void partitionFunction(
    CSR                        *csr,
    int                        &blkNum,
    cl_context                 &context,
    subPartitionDescriptor     *subPartitions
)
{

    int *rpa = (int*)get_host_mem_pointer(MEM_ID_RPA);
    int *cia = (int*)get_host_mem_pointer(MEM_ID_CIA);
    int *vertexScoreCached    = (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE_CACHED);
    int *vertexScore          = (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE);
    unsigned int *vertexMap   = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_MAP);
    unsigned int *vertexRemap = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_REMAP);

    schedulerInit(NULL);

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

    process_mem_init(context);

    blkNum =  (mapedSourceIndex + BLK_SIZE - 1) / BLK_SIZE;
    printf("blkNum:%d  vertexNum: %d \n", blkNum, vertexNum);

    for (int i = 0; i < blkNum; i ++) {
        partitionDescriptor * partition = getPartition(i);
        for (int k = 0; k < SUB_PARTITION_NUM; k++ )
        {
            partition->sub[k] = getSubPartition(i * SUB_PARTITION_NUM + k);
        }
    }

    for (int i = 0; i < blkNum; i ++) {
        partitionDescriptor * partition = getPartition(i);

        /****************************************************************************************************************/

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
        partition->totalEdge = cur_edge_num;


        for (int subIndex = 0 ; subIndex < SUB_PARTITION_NUM ; subIndex ++ )
        {
            partition->sub[subIndex]->compressRatio = (double (mapedSourceIndex)) / vertexNum;
            DEBUG_PRINTF("ratio %d / %d is %lf \n", mapedSourceIndex, vertexNum, partition->sub[subIndex]->compressRatio);
            partition->sub[subIndex]->dstVertexStart = VERTEX_MAX * (i);
            partition->sub[subIndex]->dstVertexEnd   = ((unsigned int)(VERTEX_MAX * (i + 1)) > mapedSourceIndex) ? mapedSourceIndex : VERTEX_MAX * (i + 1) ;
            volatile int subPartitionSize = ((partition->totalEdge / SUB_PARTITION_NUM) / ALIGN_SIZE) * ALIGN_SIZE;
            partition->subPartitionSize = subPartitionSize;
            partition->sub[subIndex]->srcVertexStart =  edgeScoreMap[subPartitionSize * subIndex];
            partition->sub[subIndex]->srcVertexEnd   =  edgeScoreMap[subPartitionSize * (subIndex + 1) - 1];
        }
        schedulerSubPartitionArrangement(partition);

        for (int subIndex = 0 ; subIndex < SUB_PARTITION_NUM ; subIndex ++ )
        {
            int partId = i * SUB_PARTITION_NUM + subIndex;
            int subPartitionSize = partition->subPartitionSize;
            int reOrderIndex = partition->finalOrder[subIndex];

            unsigned int bound = subPartitionSize * (reOrderIndex + 1);

            partition->sub[subIndex]->listStart =  0;
            partition->sub[subIndex]->listEnd = (bound > partition->totalEdge) ? (partition->totalEdge - (subPartitionSize * reOrderIndex)) : (subPartitionSize);
            partition->sub[subIndex]->mapedTotalIndex = mapedSourceIndex;
            partition->sub[subIndex]->srcVertexStart =  edgeScoreMap[subPartitionSize * reOrderIndex];
            partition->sub[subIndex]->srcVertexEnd   =  edgeScoreMap[subPartitionSize * (reOrderIndex + 1) - 1];

            DEBUG_PRINTF("[SIZE] %d cur_edge_num sub %d\n", partition->totalEdge, partition->sub[subIndex]->listEnd);
            partition_mem_init(context, partId, partition->sub[subIndex]->listEnd, subIndex); // subIndex --> cuIndex
            memcpy(partition->sub[subIndex]->edge.data     , &edgesTuples[subPartitionSize * reOrderIndex]  , partition->sub[subIndex]->listEnd * sizeof(int));
            memcpy(partition->sub[subIndex]->edgeMap.data  , &edgeScoreMap[subPartitionSize * reOrderIndex] , partition->sub[subIndex]->listEnd * sizeof(int));
            memcpy(partition->sub[subIndex]->edgeProp.data , &edgeProp[subPartitionSize * reOrderIndex]     , partition->sub[subIndex]->listEnd * sizeof(int));
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


        /****************************************************************************************************************/

        /****************************************************************************************************************/

    }

    schedulerPartitionArrangement(partitions, blkNum);


    for (int i = 0; i < blkNum; i++)
    {
        DEBUG_PRINTF("[SCHE] %d with %d @ %d \n", getArrangedPartitionID(i), partitions[getArrangedPartitionID(i)].totalEdge, i);
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

#if 0
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

#endif