#include "host_graph_verification_inner.h"

#include "global_config.h"
#include "fpga_application.h"




void partitionGatherScatterCModel(
    cl_context              &context,
    cl_device_id            &device,
    int                     superStep,
    int                     cuIndex,
    subPartitionDescriptor  *subPartitions
)
{
    int currentPropId = superStep % 2;
    //int resultPropId = (superStep + 1) % 2;


    int *edgesTailArray   = (int *)get_host_mem_pointer(subPartitions->edgeTail.id);
    int *edgesHeadArray   = (int *)get_host_mem_pointer(subPartitions->edgeHead.id);

    transfer_data_from_pl(context, device, getGatherScatter(0)->prop[currentPropId].id);
    prop_t *propValue           = (prop_t*)get_host_mem_pointer(getGatherScatter(0)->prop[currentPropId].id);
    prop_t *tmpVertexPropVerify = (prop_t*)get_host_mem_pointer(MEM_ID_TMP_VERTEX_VERIFY);
    prop_t *edgeProp            = (prop_t*)get_host_mem_pointer(subPartitions->edgeProp.id);
    clear_host_mem(MEM_ID_TMP_VERTEX_VERIFY);
    DEBUG_PRINTF("partition cmodel verify:\n");

    for (unsigned int i = 0; i < subPartitions->listEnd; i++)
    {
        prop_t update = 0;
        int address = (edgesTailArray[i] > ((int)subPartitions->dstVertexEnd)) ? subPartitions->dstVertexEnd : edgesTailArray[i];
        if (IS_ACTIVE_VERTEX(propValue[edgesHeadArray[2*i]]))
        {
            update = PROP_COMPUTE_STAGE0(propValue[edgesHeadArray[2*i]]);
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
                         edgesTailArray[i],
                         edgesHeadArray[2*i],
                         propValue[edgesHeadArray[2*i]],
                         edgeProp[i]);
            DEBUG_PRINTF("[DUMP-2]  %d 0x%08x 0x%08x [%d] 0x%08x\n", i,
                         propValue[i], update, address, tmpVertexPropVerify[address]);
        }

#ifdef PROBE_VERTEX
        if (edgesTailArray[i] == PROBE_VERTEX)
        {
            DEBUG_PRINTF("probe (%d): %x %x %x %x \n", edgesTailArray[i], i, edgesHeadArray[2*i], propValue[edgesHeadArray[2*i]], tmpVertexPropVerify[edgesTailArray[i]]);
        }
#endif
    }
    transfer_data_from_pl(context, device, subPartitions->tmpProp.id);


    prop_t *tmpVertexProp =  (prop_t*)get_host_mem_pointer(subPartitions->tmpProp.id);

    int error_count = 0;
    int total_count = 0;
    for (unsigned int i = subPartitions->dstVertexStart; i < subPartitions->dstVertexEnd; i ++) {
        if (tmpVertexPropVerify[i] != 0)
        {
            total_count ++;
        }
        if (tmpVertexPropVerify[i] != tmpVertexProp[i - subPartitions->dstVertexStart])
        {
            error_count++;


#ifndef SW_DEBUG
            if (error_count <= 100)
#endif
            {
                DEBUG_PRINTF("gs error %d 0x%08x hw: 0x%08x  diff 0x%08x !!!!\n", i,
                             tmpVertexPropVerify[i],
                             tmpVertexProp[i - subPartitions->dstVertexStart],
                             tmpVertexPropVerify[i] - tmpVertexProp[i - subPartitions->dstVertexStart]);
            }

        }
        else
        {
            if (DATA_DUMP)
            {
                DEBUG_PRINTF("[DUMP] gs %d 0x%08x hw: 0x%08x\n", i,  tmpVertexPropVerify[i], tmpVertexProp[i - subPartitions->dstVertexStart]);
            }
        }
    }
    DEBUG_PRINTF("[RES] error_count %d  in size %d/ %d\n", error_count, total_count,
                 subPartitions->dstVertexEnd - subPartitions->dstVertexStart + 1 );
}

