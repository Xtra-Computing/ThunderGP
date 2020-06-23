
#include "host_graph_sw.h"


#include "host_graph_scheduler.h"


#define EDEG_MEMORY_SIZE        ((edgeNum + (ALIGN_SIZE * 4) * 128) * 1)
#define VERTEX_MEMORY_SIZE      (((vertexNum - 1)/VERTEX_MAX + 1) * VERTEX_MAX)

extern void base_mem_init(cl_context &context);
extern void process_mem_init(cl_context &context);
extern void partition_mem_init(cl_context &context, int blkIndex, int size, int cuIndex);

extern int schedulerRegister(void);

int acceleratorDataPrepare(const std::string &gName, const std::string &mode, graphInfo *info)
{
    graphAccelerator * acc = getAccelerator();

    Graph* gptr = createGraph(gName, mode);
    CSR* csr    = new CSR(*gptr);
    acc->csr    = csr;
    free(gptr);

    int vertexNum = csr ->vertexNum;
    int edgeNum   = csr ->edgeNum;


    register_size_attribute(SIZE_IN_EDGE     , EDEG_MEMORY_SIZE  );
    register_size_attribute(SIZE_IN_VERTEX   , VERTEX_MEMORY_SIZE);
    register_size_attribute(SIZE_USER_DEFINE , 1                 );

    base_mem_init(acc->context);

    int *rpa        = (int*)get_host_mem_pointer(MEM_ID_RPA);
    int *cia        = (int*)get_host_mem_pointer(MEM_ID_CIA);

    int *outDeg         = (int*)get_host_mem_pointer(MEM_ID_OUT_DEG);
    int *outDegOriginal = (int*)get_host_mem_pointer(MEM_ID_OUT_DEG_ORIGIN);
    for (int i = 0; i < vertexNum; i++) {
        if (i < csr->vertexNum) { // 'vertexNum' may be aligned.
            rpa[i] = csr->rpao[i];
            outDegOriginal[i] = csr->rpao[i + 1] - csr->rpao[i];
        }
        else {
            rpa[i] = 0;
            outDegOriginal[i] = 0;
        }
    }
    rpa[vertexNum] = csr->rpao[vertexNum];
    for (int i = 0; i < edgeNum; i++) {
        cia[i] = csr->ciao[i];
    }

    /* compress vertex*/
    unsigned int *vertexMap   = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_MAP);
    unsigned int *vertexRemap = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_REMAP);

    unsigned int mapedSourceIndex = 0;

    for (int u = 0; u < vertexNum; u++) {
        int num = rpa[u + 1] - rpa[u];
        vertexMap[u] = mapedSourceIndex;
        if (num != 0)
        {
            vertexRemap[mapedSourceIndex] = u;
            outDeg[mapedSourceIndex] = num;
            mapedSourceIndex ++;
        }
    }

    info->vertexNum = vertexNum;
    info->compressedVertexNum = mapedSourceIndex;
    info->edgeNum   = edgeNum;
    info->blkNum =  (mapedSourceIndex + BLK_SIZE - 1) / BLK_SIZE;

    return 0;
}



static void partitionTransfer(graphInfo *info)
{
    graphAccelerator * acc = getAccelerator();

    DEBUG_PRINTF("%s", "transfer base mem start\n");
    double  begin = getCurrentTimestamp();

    int base_mem_id[]  = {
        MEM_ID_VERTEX_SCORE_MAPPED,
        MEM_ID_OUT_DEG,
        MEM_ID_ERROR
    };
    DEBUG_PRINTF("%s", "transfer base mem\n");
    transfer_data_to_pl(acc->context, acc->device, base_mem_id, ARRAY_SIZE(base_mem_id));
    DEBUG_PRINTF("%s", "transfer subPartitions mem\n");
    for (int i = 0; i < info->blkNum; i ++) {
        int mem_id[3];
        mem_id[0] = getSubPartition(i)->edge.id;
        mem_id[1] = getSubPartition(i)->edgeMap.id;
        mem_id[2] = getSubPartition(i)->edgeProp.id;
        transfer_data_to_pl(acc->context, acc->device, mem_id, ARRAY_SIZE(mem_id));
    }

    DEBUG_PRINTF("%s", "transfer cu mem\n");
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        int mem_id[2];
        mem_id[0] = getGatherScatter(i)->prop[0].id;
        mem_id[1] = getGatherScatter(i)->tmpProp.id;
        transfer_data_to_pl(acc->context, acc->device, mem_id, ARRAY_SIZE(mem_id));
    }

    double end =  getCurrentTimestamp();
    DEBUG_PRINTF("data transfer %lf \n", (end - begin) * 1000);
}


void partitionFunction(graphInfo *info)
{
    graphAccelerator * acc = getAccelerator();

    int *rpa = (int*)get_host_mem_pointer(MEM_ID_RPA);
    int *cia = (int*)get_host_mem_pointer(MEM_ID_CIA);
    int *vertexScoreMapped    = (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE_MAPPED);
    int *vertexScore          = (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE);
    unsigned int *vertexMap   = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_MAP);
    unsigned int *vertexRemap = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_REMAP);

    schedulerInit(NULL);

    unsigned int mapedSourceIndex = 0;
    int vertexNum = info->vertexNum;

    for (int u = 0; u < vertexNum; u++) {
        int num = rpa[u + 1] - rpa[u];
        vertexMap[u] = mapedSourceIndex;
        if (num != 0)
        {
#if CAHCE_FETCH_DEBUG
            vertexScoreMapped[mapedSourceIndex] =  mapedSourceIndex;
#else
            vertexScoreMapped[mapedSourceIndex] =  vertexScore[u];
#endif
            vertexRemap[mapedSourceIndex] = u;
            mapedSourceIndex ++;
        }
    }

    process_mem_init(acc->context);

    for (int i = 0; i < info->blkNum; i ++) {
        partitionDescriptor * partition = getPartition(i);
        for (int k = 0; k < SUB_PARTITION_NUM; k++ )
        {
            partition->sub[k] = getSubPartition(i * SUB_PARTITION_NUM + k);
        }
    }

    for (int i = 0; i < info->blkNum; i ++) {
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
            cur_edge_num ++;
        }
        partition->totalEdge = cur_edge_num;


        for (int subIndex = 0 ; subIndex < SUB_PARTITION_NUM ; subIndex ++ )
        {
            partition->sub[subIndex]->compressRatio = (double (mapedSourceIndex)) / vertexNum;
            DEBUG_PRINTF("ratio %d / %d is %lf \n", mapedSourceIndex, vertexNum, partition->sub[subIndex]->compressRatio);
            partition->sub[subIndex]->dstVertexStart = VERTEX_MAX * (i);
            partition->sub[subIndex]->dstVertexEnd   = (((unsigned int)(VERTEX_MAX * (i + 1)) > mapedSourceIndex) ? mapedSourceIndex : VERTEX_MAX * (i + 1)) - 1;
            volatile int subPartitionSize = ((partition->totalEdge / SUB_PARTITION_NUM) / ALIGN_SIZE) * ALIGN_SIZE;
            partition->subPartitionSize = subPartitionSize;
            partition->sub[subIndex]->srcVertexStart =  edgeScoreMap[subPartitionSize * subIndex];
            partition->sub[subIndex]->srcVertexEnd   =  edgeScoreMap[subPartitionSize * (subIndex + 1) - 1];
        }
        schedulerSubPartitionArrangement(i);

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
            partition_mem_init(acc->context, partId, partition->sub[subIndex]->listEnd, subIndex); // subIndex --> cuIndex
            memcpy(partition->sub[subIndex]->edge.data     , &edgesTuples[subPartitionSize * reOrderIndex]  , partition->sub[subIndex]->listEnd * sizeof(int));
            memcpy(partition->sub[subIndex]->edgeMap.data  , &edgeScoreMap[subPartitionSize * reOrderIndex] , partition->sub[subIndex]->listEnd * sizeof(int));
            memcpy(partition->sub[subIndex]->edgeProp.data , &edgeProp[subPartitionSize * reOrderIndex]     , partition->sub[subIndex]->listEnd * sizeof(int));
        }
    }

#define CACHE_UNIT_SIZE (4096 * 2)


    schedulerPartitionArrangement(info->blkNum);


    for (int i = 0; i < info->blkNum; i++)
    {
        subPartitionDescriptor * subPartition = getSubPartition(i);
        int *edgeScoreMap = (int*)subPartition->edgeMap.data;

        DEBUG_PRINTF("\n----------------------------------------------------------------------------------\n");
        DEBUG_PRINTF("[PART] subPartitions %d info :\n", i);
        DEBUG_PRINTF("[PART] \t edgelist from %d to %d\n"   , subPartition->listStart      , subPartition->listEnd     );
        DEBUG_PRINTF("[PART] \t dst. vertex from %d to %d\n", subPartition->dstVertexStart , subPartition->dstVertexEnd);
        DEBUG_PRINTF("[PART] \t src. vertex from %d to %d\n", subPartition->srcVertexStart , subPartition->srcVertexEnd);
        DEBUG_PRINTF("[PART] \t dump: %d - %d\n", (edgeScoreMap[subPartition->listStart]), (edgeScoreMap[subPartition->listEnd - 1]));
        DEBUG_PRINTF("[PART] scatter cache ratio %lf \n", subPartition->scatterCacheRatio);
        DEBUG_PRINTF("[PART] v/e %lf \n", (subPartition->dstVertexEnd - subPartition->dstVertexStart)
                     / ((float)(subPartition->listEnd - subPartition->listStart)));

        DEBUG_PRINTF("[PART] v: %d e: %d \n", (subPartition->dstVertexEnd - subPartition->dstVertexStart),
                     (subPartition->listEnd - subPartition->listStart));

        DEBUG_PRINTF("[PART] est. efficient %lf\n", ((float)(subPartition->listEnd - subPartition->listStart)) / mapedSourceIndex);

        DEBUG_PRINTF("[PART] compressRatio %lf \n\n", subPartition->compressRatio);


        /****************************************************************************************************************/

        /****************************************************************************************************************/

    }


    for (int i = 0; i < info->blkNum; i++)
    {
        DEBUG_PRINTF("[SCHE] %d with %d @ %d \n", getArrangedPartitionID(i), getPartition(getArrangedPartitionID(i))->totalEdge, i);
    }

    int paddingVertexNum  = ((vertexNum  - 1) / (ALIGN_SIZE * 4) + 1 ) * (ALIGN_SIZE * 4);
    int *tmpVertexProp =  (int*)get_host_mem_pointer(MEM_ID_TMP_VERTEX_PROP);
    for (int i = vertexNum; i < paddingVertexNum; i++)
    {
        tmpVertexProp[i] = 0;
    }
    partitionTransfer(info);
}

int acceleratorDataPreprocess(graphInfo *info)
{
    schedulerRegister();
    dataPrepareProperty(info);
    partitionFunction(info);
    return 0;
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