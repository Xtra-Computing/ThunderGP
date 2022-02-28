
#include "host_graph_verification_inner.h"

#include "global_config.h"
#include "fpga_application.h"


#if  CUSTOMIZE_APPLY == 0




void partitionApplyCModel(
    cl_context              &context,
    cl_device_id            &device,
    int                     superStep,
    int                     partId,
    unsigned int            applyArg
)
{
    int currentPropId = superStep % 2;
    int resultPropId = (superStep + 1) % 2;
    unsigned int applyArgReg = applyArg;

    unsigned int infoArrayVerify[APPLY_REF_ARRAY_SIZE];

    for (int i = 0; i < APPLY_REF_ARRAY_SIZE; i++)
    {
        infoArrayVerify[i] = 0;
    }


    for (int i  = 0; i < SUB_PARTITION_NUM; i++)
    {
        transfer_data_from_pl(context, device, getSubPartition(partId * SUB_PARTITION_NUM + i)->tmpProp.id);

    }
    prop_t * pCuData[SUB_PARTITION_NUM];
    prop_t * updateVerify        = (prop_t*)get_host_mem_pointer(MEM_ID_VERTEX_PROP_VERIFY);
    prop_t * outDeg              = (prop_t*)get_host_mem_pointer(MEM_ID_OUT_DEG);

    transfer_data_from_pl(context, device, getGatherScatter(0)->prop[currentPropId].id);
    prop_t * propValue           = (prop_t *)get_host_mem_pointer(getGatherScatter(0)->prop[currentPropId].id);

    subPartitionDescriptor  *p_partition = getSubPartition(partId * SUB_PARTITION_NUM);

    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        pCuData[i] = (prop_t*)get_host_mem_pointer(getSubPartition(partId * SUB_PARTITION_NUM + i)->tmpProp.id);
    }

    volatile unsigned int partitionVertexNum = ((p_partition->dstVertexEnd - p_partition->dstVertexStart)
            / (ALIGN_SIZE) + 1 ) * (ALIGN_SIZE);
    DEBUG_PRINTF("[DUMP] partitionVertexNum %d\n", partitionVertexNum);

    int offset = getSubPartition(partId * SUB_PARTITION_NUM)->dstVertexStart;

    for (int i = 0; i < MAX_VERTICES_IN_ONE_PARTITION; i++)
    {
        prop_t mergeData = 0;
        for (int j = 0 ; j < SUB_PARTITION_NUM; j++)
        {
            prop_t* pLocal = pCuData[j];

            mergeData = PROP_COMPUTE_STAGE4(mergeData, pLocal[i]);
            if (DATA_DUMP)
            {
                DEBUG_PRINTF("[DUMP-0] index: %d, pLocal[i]: 0x%08x, mergeData: 0x%08x \n", i,
                             pLocal[i], mergeData);
            }
        }

        prop_t tProp = mergeData;
    
        updateVerify[i] = applyFunc(tProp, propValue[i + offset], outDeg[i + offset], infoArrayVerify, *(unsigned int *)&applyArgReg);
        if (DATA_DUMP)
        {
            DEBUG_PRINTF("[DUMP-0] index: %d, tmpVertexProp: 0x%08x, propValue[i + offset]: 0x%08x, outDeg[i + offset]: 0x%08x, updateVerify[i]: 0x%08x \n", i,
                            tProp, propValue[i + offset], outDeg[i + offset], updateVerify[i]);
        }
#if 0
        int dump_flag = 0;
        for (int k = 0 ; k < APPLY_REF_ARRAY_SIZE; k++)
        {
            if (infoArrayVerify[k] != 0)
            {
                dump_flag = 1;
            }
        }
        if (dump_flag)
        {
            //if (i == 18)
            {
                DEBUG_PRINTF("[DUMP-0] %d 0x%08x 0x%08x 0x%08x \n", i,
                             tProp, propValue[i + offset], updateVerify[i]);
            }

        }
#endif

    }

    int error_count = 0;
    transfer_data_from_pl(context, device, getGatherScatter(0)->prop[resultPropId].id);
    prop_t* hwUpdate = (prop_t *)get_host_mem_pointer(getGatherScatter(0)->prop[resultPropId].id);
    for (unsigned int i = 0; i < p_partition->dstVertexEnd - p_partition->dstVertexStart + 1; i++)
    {
        if (updateVerify[i] !=  hwUpdate[i + offset])
        {
            error_count ++;
            if (error_count < 50)
            {

                DEBUG_PRINTF("apply error %d 0x%08x hw: 0x%08x  diff 0x%08x !!!!\n", i,
                             updateVerify[i],
                             hwUpdate[i + offset],
                             updateVerify[i] - hwUpdate[i + offset]);
            }
        }
        if (DATA_DUMP)
        {
            DEBUG_PRINTF("[DUMP] %d, updateVerify[i]: 0x%08x, hwUpdate[i + offset]: 0x%08x,  diff 0x%08x \n", i,
                         updateVerify[i],
                         hwUpdate[i + offset],
                         updateVerify[i] - hwUpdate[i + offset]);
        }
    }
    DEBUG_PRINTF("[RES] apply error_count %d \n", error_count);
}

#endif