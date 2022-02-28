#include "host_graph_verification_inner.h"
#include "host_graph_scheduler.h"


#include "global_config.h"
#include "fpga_application.h"




int acceleratorProfile (int superStep, int runCounter, graphInfo *info, double exeTime)
{
    graphAccelerator * acc = getAccelerator();
    int blkNum = info->blkNum;
    for (int i = 0; i < blkNum; i ++)
    {
        partitionDescriptor * partition = getPartition(i);
#if HAVE_APPLY

        unsigned long applyTime = xcl_get_event_duration(partition->applyEvent);
#else
        unsigned long applyTime = 0;
#endif
        for (int j = 0; j < SUB_PARTITION_NUM; j ++)
        {
            unsigned long fpgaTime = xcl_get_event_duration(partition->syncEvent[j]);
            partition->sub[j]->log.fpgaExeTime = fpgaTime;
        }
        partition->applyExeTime = applyTime;

    }

    /* verification */
    //if (runCounter == 0)
    {
        for (int i = 0; i < blkNum; i ++)
        {
            partitionDescriptor * partition = getPartition(i);
            for (int j = 0; j < SUB_PARTITION_NUM; j ++)
            {
                partitionGatherScatterCModel(acc->context, acc->device, superStep, j, partition->sub[j]);
            }
#if HAVE_APPLY
            partitionApplyCModel(acc->context, acc->device, superStep, getArrangedPartitionID(i), dataPrepareGetArg(info));
#endif
        }

    }
    /* log */
    if (runCounter > 1)
    {
        double fpga_runtime_total = 0;
        double end2end_runtime_total = 0;
        for (int i = 0;  i < blkNum; i++)
        {
            double max_fpga_exe = 0;
            partitionDescriptor * partition = getPartition(i);
            for (int j = 0; j < SUB_PARTITION_NUM; j ++)
            {
                if (partition->sub[j]->log.fpgaExeTime > max_fpga_exe)
                {
                    max_fpga_exe = partition->sub[j]->log.fpgaExeTime ;
                }
                DEBUG_PRINTF("[INFO] part_gs %d cu%d exe: %f \n", i, j, partition->sub[j]->log.fpgaExeTime / 1000000.0);
            }
            end2end_runtime_total = exeTime * 1000;
            fpga_runtime_total    += max_fpga_exe / 1000000.0;
            DEBUG_PRINTF("[INFO] part_apply %d exe: %f \n", i, partition->applyExeTime / 1000000.0);
#if 0
            DEBUG_PRINTF("[INFO] partedge %f fpga gs: %f ms, apply: %f ms %d, effic %lf  v/e %lf compress %lf \n",
                         exeTime,
                         max_fpga_exe / 1000000.0,

                         partition->subPartitionSize,
                         (((float)partition->subPartitionSize) / subPartitions[i * SUB_PARTITION_NUM].mapedTotalIndex),
                         ((subPartitions[i * SUB_PARTITION_NUM].dstVertexEnd - subPartitions[i * SUB_PARTITION_NUM].dstVertexStart)
                          / ((float)(subPartitions[i * SUB_PARTITION_NUM].listEnd - subPartitions[i * SUB_PARTITION_NUM].listStart))),
                         subPartitions[i * SUB_PARTITION_NUM].compressRatio);
#endif

        }
        DEBUG_PRINTF("[INFO] summary e2e %lf ms, fpga %lf ms\n",
                     end2end_runtime_total,
                     fpga_runtime_total)
    }
    return 0;
}


int acceleratorCModelDataPreprocess(graphInfo *info)
{
    prop_t *vertexPushinPropMapped = (prop_t*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP_MAPPED);
    prop_t *propValue              = (prop_t*)get_host_mem_pointer(MEM_ID_HOST_PROP_PING);

    memcpy(propValue, vertexPushinPropMapped, (get_he_mem(MEM_ID_TMP_VERTEX_VERIFY)->size));
    return 0;
}


int acceleratorCModelSuperStep(int superStep, graphInfo *info)
{
#if HAVE_APPLY
#if (CUSTOMIZE_APPLY == 0)
    graphAccelerator * acc = getAccelerator();
    int resultPropId = (superStep + 1) % 2;
    int *rpa = (int*)get_host_mem_pointer(MEM_ID_RPA);
    int *cia = (int*)get_host_mem_pointer(MEM_ID_CIA);

    unsigned int *vertexMap        = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_MAP);
    //unsigned int *vertexRemap      = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_REMAP);
    unsigned int applyArgReg       = dataPrepareGetArg(info);

    prop_t *propValue              = (prop_t*)get_host_mem_pointer(MEM_ID_HOST_PROP_PING);
    prop_t *updateVerify           = (prop_t*)get_host_mem_pointer(MEM_ID_HOST_PROP_PONG);
#if HAVE_EDGE_PROP
    prop_t *edgeProp               = (prop_t*)get_host_mem_pointer(MEM_ID_EDGE_PROP);
#endif
    prop_t *tmpVertexPropVerify    = (prop_t*)get_host_mem_pointer(MEM_ID_TMP_VERTEX_VERIFY);
    prop_t *outDeg                 = (prop_t*)get_host_mem_pointer(MEM_ID_OUT_DEG);

    clear_host_mem(MEM_ID_TMP_VERTEX_VERIFY);


    int mapedSourceIndex = 0;
    int vertexNum = info->vertexNum;

    unsigned int infoArrayVerify[APPLY_REF_ARRAY_SIZE];

    for (int i = 0; i < APPLY_REF_ARRAY_SIZE; i++)
    {
        infoArrayVerify[i] = 0;
    }


    for (int u = 0; u < vertexNum; u++) {
        int start = rpa[u];
        int num = rpa[u + 1] - rpa[u];
        for (int j = 0; j < num; j++) {
            int cia_idx = start + j;
            int vertex_idx = vertexMap[cia[cia_idx]];
            prop_t update = 0;
            if (IS_ACTIVE_VERTEX(propValue[mapedSourceIndex]))
            {
                update = PROP_COMPUTE_STAGE0(propValue[mapedSourceIndex]);
#if HAVE_EDGE_PROP
                update = PROP_COMPUTE_STAGE1(update, edgeProp[cia_idx]);
#else
                update = PROP_COMPUTE_STAGE1(update, 0);
#endif
                tmpVertexPropVerify[vertex_idx] = PROP_COMPUTE_STAGE3(tmpVertexPropVerify[vertex_idx], update);
                int i = vertex_idx;
                if (DATA_DUMP)
                {
                    DEBUG_PRINTF("[DUMP-4] vertex_idx: %d, mapedSourceIndex: %d, propValue[mapedSourceIndex]: 0x%08x, tmpVertexPropVerify[vertex_idx]: 0x%08x \n", i,
                                    mapedSourceIndex, propValue[mapedSourceIndex], tmpVertexPropVerify[vertex_idx]);
                }
            }
        }
        if (num != 0)
        {
            mapedSourceIndex ++;
        }
    }
    for (int u = 0; u < vertexNum; u++)
    {
        updateVerify[u] = applyFunc(tmpVertexPropVerify[u],
                                           propValue[u], outDeg[u], infoArrayVerify, *(unsigned int *)&applyArgReg);
        if (DATA_DUMP)
        {
            DEBUG_PRINTF("[DUMP-3] vertexID: %d, tmpVertexPropVerify[u]: 0x%08x, propValue[u]: 0x%08x, outDeg[u]: 0x%08x, updateVerify[u]: 0x%08x \n", u,
                            tmpVertexPropVerify[u], propValue[u], outDeg[u], updateVerify[u]);
        }
    }

    int error_count = 0;
    transfer_data_from_pl(acc->context, acc->device, getGatherScatter(0)->prop[resultPropId].id);
    prop_t* hwUpdate = (prop_t *)get_host_mem_pointer(getGatherScatter(0)->prop[resultPropId].id);
    for (int i = 0; i < vertexNum; i++)
    {
        if (updateVerify[i] !=  hwUpdate[i])
        {
            error_count ++;
            if (error_count < 50)
            {

                DEBUG_PRINTF("cmodel error %d 0x%08x hw: 0x%08x  diff 0x%08x !!!!\n", i,
                             updateVerify[i],
                             hwUpdate[i],
                             updateVerify[i] - hwUpdate[i]);
            }
        }
    }
    DEBUG_PRINTF("total cmodel error: %d\n", error_count);
    memcpy(propValue, updateVerify, (get_he_mem(MEM_ID_HOST_PROP_PONG)->size));
#endif
#endif
    return 0;
}