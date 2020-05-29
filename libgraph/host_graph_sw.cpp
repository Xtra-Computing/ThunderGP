
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

#include "mem_config.h"


#define EDEG_MEMORY_SIZE        ((edgeNum + (ALIGN_SIZE * 4) * blkNum) * 1)
#define VERTEX_MEMORY_SIZE      (((vertexNum - 1)/VERTEX_MAX + 1) * VERTEX_MAX)



partitionDescriptor partitions[MAX_PARTITIONS_NUM];
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

partitionDescriptor * getPartition(int partID)
{
    return &partitions[partID];
}


gatherScatterDescriptor * getGatherScatter(int kernelID)
{
    return &localGsKernel[kernelID];
}



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
    partitionDescriptor *partitionItem  = &partitions[i];
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
    for (int i = 0; i < (vertexNum/ (ALIGN_SIZE ) + 1) * (ALIGN_SIZE ); i++) {
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
    CSR                     *csr,
    int                     &blkNum,
    cl_context              &context,
    partitionDescriptor     *partitions
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

            partitions[i].scatterCacheRatio =  ((float)hit_counter) / ((mapedSourceIndex / DELTA + 1 ));
            DEBUG_PRINTF("[TRS] scatter cache ratio: %lf in %d @blk %d \n",
                         partitions[i].scatterCacheRatio,
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
        for (int subIndex = 0 ; subIndex < SUB_PARTITION_NUM ; subIndex ++ )
        {
            int  partId = i * SUB_PARTITION_NUM + subIndex;
            partitions[partId].compressRatio = (double (mapedSourceIndex)) / vertexNum;
            DEBUG_PRINTF("ratio %d / %d is %lf \n", mapedSourceIndex, vertexNum, partitions[partId].compressRatio);
            partitions[partId].dstVertexStart = VERTEX_MAX * (i);
            partitions[partId].dstVertexEnd   = (((unsigned int)(VERTEX_MAX * (i + 1)) > mapedSourceIndex) ? mapedSourceIndex : (VERTEX_MAX * (i + 1)  - 1));
            volatile int subPartitionSize = ((cur_edge_num / SUB_PARTITION_NUM) / ALIGN_SIZE) * ALIGN_SIZE;
            int reOrderIndex = 0;
            if (i % 2 == 0)
            {
                reOrderIndex = subIndex;
            }
            else
            {
                reOrderIndex = SUB_PARTITION_NUM - subIndex - 1;
            }
            unsigned int bound = subPartitionSize * (reOrderIndex + 1);
            partitions[partId].listStart =  0;
            partitions[partId].listEnd = (bound > cur_edge_num) ? (cur_edge_num - (subPartitionSize * reOrderIndex)) : (subPartitionSize);
            partitions[partId].mapedTotalIndex = mapedSourceIndex;
            DEBUG_PRINTF("[SIZE] %d cur_edge_num sub %d\n", cur_edge_num, partitions[partId].listEnd);
            partition_mem_init(context, partId, partitions[partId].listEnd, subIndex); // subIndex --> cuIndex
            memcpy(partitions[partId].edge.data     , &edgesTuples[subPartitionSize * reOrderIndex]  , partitions[partId].listEnd * sizeof(int));
            memcpy(partitions[partId].edgeMap.data  , &edgeScoreMap[subPartitionSize * reOrderIndex] , partitions[partId].listEnd * sizeof(int));
            memcpy(partitions[partId].edgeProp.data , &edgeProp[subPartitionSize * reOrderIndex]     , partitions[partId].listEnd * sizeof(int));
        }
    }

#define CACHE_UNIT_SIZE (4096 * 2)
    for (int index = 0; index < blkNum; index++)
    {
        int i = index * SUB_PARTITION_NUM;
        int *edgeScoreMap = (int*)partitions[i].edgeMap.data;
        DEBUG_PRINTF("\n----------------------------------------------------------------------------------\n");
        DEBUG_PRINTF("[PART] partitions %d info :\n", i);
        DEBUG_PRINTF("[PART] \t edgelist from %d to %d\n"   , partitions[i].listStart      , partitions[i].listEnd     );
        DEBUG_PRINTF("[PART] \t dst. vertex from %d to %d\n", partitions[i].dstVertexStart , partitions[i].dstVertexEnd);
        DEBUG_PRINTF("[PART] \t src. vertex from %d to %d\n", partitions[i].srcVertexStart , partitions[i].srcVertexEnd);
        DEBUG_PRINTF("[PART] \t dump: %d - %d\n", (edgeScoreMap[partitions[i].listStart]), (edgeScoreMap[partitions[i].listEnd - 1]));
        DEBUG_PRINTF("[PART] scatter cache ratio %lf \n", partitions[i].scatterCacheRatio);
        DEBUG_PRINTF("[PART] v/e %lf \n", (partitions[i].dstVertexEnd - partitions[i].dstVertexStart)
                     / ((float)(partitions[i].listEnd - partitions[i].listStart)));

        DEBUG_PRINTF("[PART] v: %d e: %d \n", (partitions[i].dstVertexEnd - partitions[i].dstVertexStart),
                     (partitions[i].listEnd - partitions[i].listStart));

        DEBUG_PRINTF("[PART] est. efficient %lf\n", ((float)(partitions[i].listEnd - partitions[i].listStart)) / mapedSourceIndex);

        DEBUG_PRINTF("[PART] compressRatio %lf \n\n", partitions[i].compressRatio);


#if SRC_VERTEX_CHECK
        int lastMapIndex = edgeScoreMap[partitions[i].listStart];
        int inner_counter = 0;
        DEBUG_PRINTF("SRC_VERTEX_CHECK @%d\n", SRC_VERTEX_CHECK_UNIT);
        int max_diff  = 0;
        int diff_count[3] = {0};
        for (unsigned int j = partitions[i].listStart;  j < partitions[i].listEnd; j += SRC_VERTEX_CHECK_UNIT)
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
    for (int i = 0; i < blkNum; i++)
    {
        for (int j = 0; j < blkNum - i - 1; j++)
        {
            if (partitions[partIdTable[j] * SUB_PARTITION_NUM].listEnd < partitions[partIdTable[j + 1] * SUB_PARTITION_NUM].listEnd)
            {
                int tmpId = partIdTable[j];
                partIdTable[j] = partIdTable[j + 1];
                partIdTable[j + 1]  = tmpId;
            }
        }
    }
    for (int i = 0; i < blkNum; i++)
    {
        DEBUG_PRINTF("[SCHE] %d with %d @ %d \n", partIdTable[i], partitions[partIdTable[i] * SUB_PARTITION_NUM].listEnd, i);
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




int float2int(float a) {
    return (int)(a * INT2FLOAT);
}

float int2float(int a) {
    return ((float)a / INT2FLOAT);
}

double getCurrentTimestamp(void) {
    timespec a;
    clock_gettime(CLOCK_MONOTONIC, &a);
    return (double(a.tv_nsec) * 1.0e-9) + double(a.tv_sec);
}

