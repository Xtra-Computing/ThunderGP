#include "host_graph_sw.h"

#include "config.h"
#include "fpga_application.h"

#include "host_graph_sw_verification.h"


void partitionGatherScatterCModel(
    cl_context              &context,
    cl_device_id            &device,
    int                     cuIndex,
    partitionDescriptor     *partitions
)
{
    unsigned int  *edgesTuples   = (unsigned int *)get_host_mem_pointer(partitions->edge.id);
    int *edgeScoreMap        = (int*)get_host_mem_pointer(partitions->edgeMap.id);
    prop_t *vertexScore         = (prop_t*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE_CACHED);
    prop_t *tmpVertexPropVerify = (prop_t*)get_host_mem_pointer(MEM_ID_TMP_VERTEX_VERIFY);
    prop_t *edgeProp            = (prop_t*)get_host_mem_pointer(partitions->edgeProp.id);
    clear_host_mem(MEM_ID_TMP_VERTEX_VERIFY);
    DEBUG_PRINTF("partition cmodel verify:\n");

    for (unsigned int i = 0; i < partitions->listEnd; i++)
    {
        prop_t update = 0;
        unsigned int address = (edgesTuples[i] > partitions->dstVertexEnd) ? partitions->dstVertexEnd : edgesTuples[i];
        if (IS_ACTIVE_VERTEX(vertexScore[edgeScoreMap[i]]))
        {
            update = PROP_COMPUTE_STAGE0(vertexScore[edgeScoreMap[i]]);
#if HAVE_EDGE_PROP
            update = PROP_COMPUTE_STAGE1(update, edgeProp[i]);
#else
            update = PROP_COMPUTE_STAGE1(update, 0);
#endif
            tmpVertexPropVerify[address] = PROP_COMPUTE_STAGE3(tmpVertexPropVerify[address], update);
        }
        if (DATA_DUMP)
        {
            DEBUG_PRINTF("[DUMP]  %d 0x%08x-->0x%08x 0x%08x with 0x%08x \n", i,
                         edgesTuples[i],
                         edgeScoreMap[i],
                         vertexScore[edgeScoreMap[i]],
                         edgeProp[i]);
            DEBUG_PRINTF("[DUMP-2]  %d 0x%08x 0x%08x [%d] 0x%08x\n", i,
                         vertexScore[i], update, address, tmpVertexPropVerify[address]);
        }

#ifdef PROBE_VERTEX
        if (edgesTuples[i] == PROBE_VERTEX)
        {
            DEBUG_PRINTF("probe (%d): %d %d %d %d \n", edgesTuples[i], i, edgeScoreMap[i], vertexScore[edgeScoreMap[i]], tmpVertexPropVerify[edgesTuples[i]]);
        }
#endif
    }
    transfer_data_from_pl(context, device, partitions->tmpProp.id);


    prop_t *tmpVertexProp =  (prop_t*)get_host_mem_pointer(partitions->tmpProp.id);

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
#endif
            {
                DEBUG_PRINTF("error tmp  %d 0x%08x 0x%08x  diff 0x%08x !!!!\n", i,
                             tmpVertexProp[i],
                             tmpVertexPropVerify[i],
                             tmpVertexPropVerify[i] - tmpVertexProp[i]);
            }

        }
        else
        {
            if (DATA_DUMP)
            {
                DEBUG_PRINTF("[DUMP] tmp  %d 0x%08x 0x%08x\n", i, tmpVertexProp[i], tmpVertexPropVerify[i]);
            }
        }
    }
    DEBUG_PRINTF("[RES] error_count %d  in size %d/ %d\n", error_count, total_count,
                 partitions->dstVertexEnd - partitions->dstVertexStart + 1 );
}

