
#include "host_graph_sw.h"

#include "host_graph_mem.h"

#include "host_graph_scheduler.h"


#define EDEG_MEMORY_SIZE        ((edgeNum + (ALIGN_SIZE * 4) * 128) * 1)
#define VERTEX_MEMORY_SIZE      (((vertexNum - 1)/MAX_VERTICES_IN_ONE_PARTITION + 1) * MAX_VERTICES_IN_ONE_PARTITION)


int acceleratorDataLoad(const std::string &gName, const std::string &mode, graphInfo *info)
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

    base_mem_init(acc->context); // initialize all the he_mems. 

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
    info->blkNum =  (mapedSourceIndex + PARTITION_SIZE - 1) / PARTITION_SIZE;

    return 0;
}



static void partitionTransfer(graphInfo *info)
{
    graphAccelerator * acc = getAccelerator();

    DEBUG_PRINTF("%s", "transfer base mem start\n");
    double  begin = getCurrentTimestamp();

    int base_mem_id[]  = {
        MEM_ID_PUSHIN_PROP_MAPPED,
        MEM_ID_OUT_DEG,
        MEM_ID_RESULT_REG
    };
    DEBUG_PRINTF("%s", "transfer base mem\n");
    transfer_data_to_pl(acc->context, acc->device, base_mem_id, ARRAY_SIZE(base_mem_id));
    DEBUG_PRINTF("%s", "transfer subPartitions mem\n");
    for (int i = 0; i < info->blkNum; i ++) {
        for (int j = 0; j < SUB_PARTITION_NUM; j++) {
            int mem_id[2];
            //mem_id[0] = getSubPartition(i)->edgeTail.id;
            mem_id[0] = getSubPartition(i * SUB_PARTITION_NUM + j)->edgeHead.id;
            mem_id[1] = getSubPartition(i * SUB_PARTITION_NUM + j)->edgeProp.id;
            transfer_data_to_pl(acc->context, acc->device, mem_id, ARRAY_SIZE(mem_id));   
        }
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

void reTransferProp(graphInfo *info)
{
    dataPrepareProperty(info);
    graphAccelerator * acc = getAccelerator();

    int *rpa = (int*)get_host_mem_pointer(MEM_ID_RPA);
    prop_t *vertexPushinPropMapped = (prop_t*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP_MAPPED);
    prop_t *vertexPushinProp       = (prop_t*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP);
    unsigned int *vertexMap        = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_MAP);
    unsigned int *vertexRemap      = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_REMAP);
    unsigned int mapedSourceIndex  = 0;
    int vertexNum = info->vertexNum;

    for (int u = 0; u < vertexNum; u++) {
        int num = rpa[u + 1] - rpa[u];
        vertexMap[u] = mapedSourceIndex;
        if (num != 0)
        {
#if CAHCE_FETCH_DEBUG
            vertexPushinPropMapped[mapedSourceIndex] =  mapedSourceIndex;
#else
            vertexPushinPropMapped[mapedSourceIndex] =  vertexPushinProp[u];
#endif
            vertexRemap[mapedSourceIndex] = u;
            mapedSourceIndex ++;
        }
    }
    process_mem_init(acc->context);
    partitionTransfer(info);
}


void partitionFunction(graphInfo *info)
{
    graphAccelerator * acc = getAccelerator();

    int *rpa = (int*)get_host_mem_pointer(MEM_ID_RPA);
    int *cia = (int*)get_host_mem_pointer(MEM_ID_CIA);
    prop_t *vertexPushinPropMapped = (prop_t*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP_MAPPED);
    prop_t *vertexPushinProp       = (prop_t*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP);
    unsigned int *vertexMap        = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_MAP);
    unsigned int *vertexRemap      = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_REMAP);

    schedulerInit(NULL);

    unsigned int mapedSourceIndex = 0;
    int vertexNum = info->vertexNum;

    for (int u = 0; u < vertexNum; u++) {
        int num = rpa[u + 1] - rpa[u];
        vertexMap[u] = mapedSourceIndex;
        if (num != 0)
        {
#if CAHCE_FETCH_DEBUG
            vertexPushinPropMapped[mapedSourceIndex] =  mapedSourceIndex;
#else
            vertexPushinPropMapped[mapedSourceIndex] =  vertexPushinProp[u];
#endif
            vertexRemap[mapedSourceIndex] = u;
            mapedSourceIndex ++;
        }
    }

    process_mem_init(acc->context); // initialize all gs kernel memories.

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
        int *edgePartitionTailArray    = (int*)get_host_mem_pointer(MEM_ID_EDGE_TAIL);
        int *edgePartitionHeadArray    = (int*)get_host_mem_pointer(MEM_ID_EDGE_HEAD);
        prop_t *edgePartitionPropArray = (prop_t*)get_host_mem_pointer(MEM_ID_PARTITON_EDGE_PROP);
        prop_t *edgeProp               = (prop_t*)get_host_mem_pointer(MEM_ID_EDGE_PROP);
        mapedSourceIndex = 0;

        for (int u = 0; u < vertexNum; u++) {
            int start = rpa[u];
            int num = rpa[u + 1] - rpa[u];
            for (int j = 0; j < num; j++) {
                //tmpVertexProp[cia[start + j]] += vertexPushinProp[u];//vertexProp[u] / (csr->rpao[u+1] - csr->rpao[u]);
                int cia_idx = start + j; //printf("cia_idx %d\n",cia_idx );
                int vertex_idx = vertexMap[cia[cia_idx]];
                if ((vertex_idx >= i * MAX_VERTICES_IN_ONE_PARTITION) && (vertex_idx < (i + 1) * MAX_VERTICES_IN_ONE_PARTITION)) {
                    edgePartitionTailArray[cur_edge_num] = vertex_idx;
                    edgePartitionHeadArray[cur_edge_num] = mapedSourceIndex;
                    edgePartitionPropArray[cur_edge_num] = edgeProp[cia_idx];
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
            edgePartitionTailArray[cur_edge_num] = (ENDFLAG - 1);
            edgePartitionHeadArray[cur_edge_num] = edgePartitionHeadArray[cur_edge_num - 1];
            edgePartitionPropArray[cur_edge_num] = 0;
            cur_edge_num ++;
        }
        partition->totalEdge = cur_edge_num;


        for (int subIndex = 0 ; subIndex < SUB_PARTITION_NUM ; subIndex ++ )
        {
            partition->sub[subIndex]->compressRatio = (double (mapedSourceIndex)) / vertexNum;
            DEBUG_PRINTF("ratio %d / %d is %lf \n", mapedSourceIndex, vertexNum, partition->sub[subIndex]->compressRatio);
            partition->sub[subIndex]->dstVertexStart = MAX_VERTICES_IN_ONE_PARTITION * (i);
            partition->sub[subIndex]->dstVertexEnd   = (((unsigned int)(MAX_VERTICES_IN_ONE_PARTITION * (i + 1)) > mapedSourceIndex) ? mapedSourceIndex : MAX_VERTICES_IN_ONE_PARTITION * (i + 1)) - 1;
            volatile int subPartitionSize = ((partition->totalEdge / SUB_PARTITION_NUM) / ALIGN_SIZE) * ALIGN_SIZE; //3998 10 4 = 396
            partition->subPartitionSize = subPartitionSize;
            partition->sub[subIndex]->srcVertexStart =  edgePartitionHeadArray[subPartitionSize * subIndex];
            partition->sub[subIndex]->srcVertexEnd   =  edgePartitionHeadArray[subPartitionSize * (subIndex + 1) - 1];
        }
        schedulerSubPartitionArrangement(i);

        for (int subIndex = 0 ; subIndex < SUB_PARTITION_NUM ; subIndex ++ )
        {
            int partId = i * SUB_PARTITION_NUM + subIndex;
            int subPartitionSize = partition->subPartitionSize;
            int reOrderIndex = partition->finalOrder[subIndex];

            unsigned int bound = subPartitionSize * (reOrderIndex + 1) + SUB_PARTITION_NUM * ALIGN_SIZE; 

            partition->sub[subIndex]->listStart =  0; 
            partition->sub[subIndex]->listEnd = (bound > partition->totalEdge && reOrderIndex == (SUB_PARTITION_NUM - 1)) ? (partition->totalEdge - (subPartitionSize * reOrderIndex)) : (subPartitionSize);
            partition->sub[subIndex]->mapedTotalIndex = mapedSourceIndex;
            partition->sub[subIndex]->srcVertexStart =  edgePartitionHeadArray[subPartitionSize * reOrderIndex];
            partition->sub[subIndex]->srcVertexEnd   =  edgePartitionHeadArray[subPartitionSize * (reOrderIndex + 1) - 1];

            DEBUG_PRINTF("[SIZE] %d cur_edge_num sub %d\n", partition->totalEdge, partition->sub[subIndex]->listEnd);
            
            partition_mem_init(acc->context, partId, partition->sub[subIndex]->listEnd, subIndex); // subIndex --> cuIndex
            
            memcpy(partition->sub[subIndex]->edgeTail.data , &edgePartitionTailArray[subPartitionSize * reOrderIndex],
               partition->sub[subIndex]->listEnd * sizeof(int));
            memcpy(partition->sub[subIndex]->edgeHead.data , &edgePartitionHeadArray[subPartitionSize * reOrderIndex], 
                partition->sub[subIndex]->listEnd * sizeof(int));

            int * pointer_edgeHeadData = (int *)(partition->sub[subIndex]->edgeHead.data);

            for (uint i = 0; i < partition->sub[subIndex]->listEnd; i ++)
            {
                pointer_edgeHeadData[2 * i] = edgePartitionHeadArray[subPartitionSize * reOrderIndex + i];
                pointer_edgeHeadData[2 * i + 1] = edgePartitionTailArray[subPartitionSize * reOrderIndex + i];
            }
            
            memcpy(partition->sub[subIndex]->edgeProp.data , &edgePartitionPropArray[subPartitionSize * reOrderIndex],
                partition->sub[subIndex]->listEnd * sizeof(int));
        }
    }

#define CACHE_UNIT_SIZE (4096 * 2)


    schedulerPartitionArrangement(info->blkNum);


    for (int i = 0; i < info->blkNum; i++)
    {
        subPartitionDescriptor * subPartition = getSubPartition(i);
        int *edgePartitionHeadArray = (int*)subPartition->edgeHead.data;

        DEBUG_PRINTF("\n----------------------------------------------------------------------------------\n");
        DEBUG_PRINTF("[PART] subPartitions %d info :\n", i);
        DEBUG_PRINTF("[PART] \t edgelist from %d to %d\n"   , subPartition->listStart      , subPartition->listEnd     );
        DEBUG_PRINTF("[PART] \t dst. vertex from %d to %d\n", subPartition->dstVertexStart , subPartition->dstVertexEnd);
        DEBUG_PRINTF("[PART] \t src. vertex from %d to %d\n", subPartition->srcVertexStart , subPartition->srcVertexEnd);
        DEBUG_PRINTF("[PART] \t dump: %d - %d\n", (edgePartitionHeadArray[subPartition->listStart]), (edgePartitionHeadArray[subPartition->listEnd - 1]));
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
