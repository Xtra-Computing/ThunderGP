#include "host_graph_sw.h"

#include "he_mem_config.h"


#define PARTITION_DDR       (he_get_attr_by_cu(cuIndex))
#define CU_DDR              (he_get_attr_by_cu(cuIndex))

void base_mem_init(cl_context &context)
{
    for (unsigned int i = 0; i < ARRAY_SIZE(local_mem); i++)
    {
        he_mem_init(context, &local_mem[i]);
    }
}

static void gs_mem_init(cl_context &context, gatherScatterDescriptor *gsItem, int cuIndex, void *data)
{
    gsItem->prop[0].id =  MEM_ID_GS_BASE + cuIndex * MEM_ID_GS_OFFSET;
    gsItem->prop[0].name = "cu prop ping";
    gsItem->prop[0].attr = CU_DDR;
    gsItem->prop[0].unit_size = sizeof(int);
    gsItem->prop[0].size_attr = SIZE_IN_VERTEX;
    he_mem_init(context, &gsItem->prop[0]);
    memcpy(gsItem->prop[0].data, data, gsItem->prop[0].size);

    gsItem->prop[1].id =  MEM_ID_GS_BASE + cuIndex * MEM_ID_GS_OFFSET + 2;
    gsItem->prop[1].name = "cu prop pong";
    gsItem->prop[1].attr = CU_DDR;
    gsItem->prop[1].unit_size = sizeof(int);
    gsItem->prop[1].size_attr = SIZE_IN_VERTEX;
    he_mem_init(context, &gsItem->prop[1]);
    memcpy(gsItem->prop[1].data, data, gsItem->prop[1].size);

    gsItem->tmpProp.id =  MEM_ID_GS_BASE + cuIndex * MEM_ID_GS_OFFSET + 1;
    gsItem->tmpProp.name = "cu output tmpProp";
    gsItem->tmpProp.attr = CU_DDR;
    gsItem->tmpProp.unit_size = sizeof(int);
    gsItem->tmpProp.size_attr = SIZE_IN_VERTEX;
    he_mem_init(context, &gsItem->tmpProp);
}


void process_mem_init(cl_context &context)
{
    int *vertexPushinPropMapped = (int*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP_MAPPED);
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        gs_mem_init(context, getGatherScatter(i), i, vertexPushinPropMapped);
    }
}

void partition_mem_init(cl_context &context, int blkIndex, int size, int cuIndex)
{
    int i = blkIndex;
    subPartitionDescriptor *partitionItem  = getSubPartition(i);
    {
        partitionItem->cuIndex = cuIndex;
        partitionItem->edgeTail.id = MEM_ID_PARTITION_BASE + i * MEM_ID_PARTITION_OFFSET;
        partitionItem->edgeTail.name = "partition edgeTail";
        partitionItem->edgeTail.attr = PARTITION_DDR;
        partitionItem->edgeTail.unit_size = size * sizeof(int);
        partitionItem->edgeTail.size_attr = SIZE_USER_DEFINE;
        he_mem_init(context, &partitionItem->edgeTail);

        partitionItem->edgeHead.id = MEM_ID_PARTITION_BASE + i * MEM_ID_PARTITION_OFFSET + 1;
        partitionItem->edgeHead.name = "partition edgeHead";
        partitionItem->edgeHead.attr = PARTITION_DDR;
        partitionItem->edgeHead.unit_size = size * sizeof(int);
        partitionItem->edgeHead.size_attr = SIZE_USER_DEFINE;
        he_mem_init(context, &partitionItem->edgeHead);

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
        partitionItem->tmpProp.unit_size = MAX_VERTICES_IN_ONE_PARTITION * sizeof(int);
        partitionItem->tmpProp.size_attr = SIZE_USER_DEFINE;
        he_mem_init(context, &partitionItem->tmpProp);
    }
}

