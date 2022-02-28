#include "host_graph_sw.h"

#include "he_mem_config.h"


// HBM Banks requirements
#define MAX_HBM_BANKCOUNT 32
#define BANK_NAME(n) n | XCL_MEM_TOPOLOGY
const int bank[MAX_HBM_BANKCOUNT] = {
    BANK_NAME(0),  BANK_NAME(1),  BANK_NAME(2),  BANK_NAME(3),  BANK_NAME(4),
    BANK_NAME(5),  BANK_NAME(6),  BANK_NAME(7),  BANK_NAME(8),  BANK_NAME(9),
    BANK_NAME(10), BANK_NAME(11), BANK_NAME(12), BANK_NAME(13), BANK_NAME(14),
    BANK_NAME(15), BANK_NAME(16), BANK_NAME(17), BANK_NAME(18), BANK_NAME(19),
    BANK_NAME(20), BANK_NAME(21), BANK_NAME(22), BANK_NAME(23), BANK_NAME(24),
    BANK_NAME(25), BANK_NAME(26), BANK_NAME(27), BANK_NAME(28), BANK_NAME(29),
    BANK_NAME(30), BANK_NAME(31)};


// #define PARTITION_DDR       (he_get_attr_by_cu(cuIndex))
// #define CU_DDR              (he_get_attr_by_cu(cuIndex))

void base_mem_init(cl_context &context)
{
    for (unsigned int i = 0; i < ARRAY_SIZE(local_mem); i++) // local_mem is in he_mem_config, where we define their attributes. 
    { 
        he_mem_init(context, &local_mem[i]);
    }
}

static void gs_mem_init(cl_context &context, gatherScatterDescriptor *gsItem, int cuIndex, void *data)
{
    gsItem->prop[0].id =  MEM_ID_GS_BASE + cuIndex * MEM_ID_GS_OFFSET;
    gsItem->prop[0].name = "cu prop ping";
    gsItem->prop[0].attr = ATTR_PL_HBM;
    gsItem->prop[0].unit_size = sizeof(int);
    gsItem->prop[0].size_attr = SIZE_IN_VERTEX;

    // modify here to change memory channel distribution
    gsItem->prop[0].ext_attr.flags = bank[2 * cuIndex + 1];

    he_mem_init(context, &gsItem->prop[0]);
    memcpy(gsItem->prop[0].data, data, gsItem->prop[0].size);

    gsItem->prop[1].id =  MEM_ID_GS_BASE + cuIndex * MEM_ID_GS_OFFSET + 2;
    gsItem->prop[1].name = "cu prop pong";
    gsItem->prop[1].attr = ATTR_PL_HBM;
    gsItem->prop[1].unit_size = sizeof(int);
    gsItem->prop[1].size_attr = SIZE_IN_VERTEX;

    gsItem->prop[1].ext_attr.flags = bank[2 * cuIndex + 1];

    he_mem_init(context, &gsItem->prop[1]);
    memcpy(gsItem->prop[1].data, data, gsItem->prop[1].size);
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
    printf("[INIT] current cuIndex is: %d.\n", cuIndex);
    int i = blkIndex;
    subPartitionDescriptor *partitionItem  = getSubPartition(i);
    {
        
        partitionItem->cuIndex = cuIndex;
        partitionItem->edgeTail.id = MEM_ID_PARTITION_BASE + i * MEM_ID_PARTITION_OFFSET;
        partitionItem->edgeTail.name = "partition edgeTail";
        partitionItem->edgeTail.attr = ATTR_HOST_ONLY;//ATTR_PL_HBM;
        partitionItem->edgeTail.unit_size = size * sizeof(int);
        partitionItem->edgeTail.size_attr = SIZE_USER_DEFINE;

        partitionItem->edgeTail.ext_attr.flags = bank[2 * cuIndex + 1];

        he_mem_init(context, &partitionItem->edgeTail);

        partitionItem->edgeHead.id = MEM_ID_PARTITION_BASE + i * MEM_ID_PARTITION_OFFSET + 1;
        partitionItem->edgeHead.name = "partition edgeHead";
        partitionItem->edgeHead.attr = ATTR_PL_HBM;
        partitionItem->edgeHead.unit_size = size * sizeof(int) * 2; printf ("the size of edge array %d \n", size * 2);
        partitionItem->edgeHead.size_attr = SIZE_USER_DEFINE;

        partitionItem->edgeHead.ext_attr.flags = bank[2 * cuIndex + 0]; 

        he_mem_init(context, &partitionItem->edgeHead);

        partitionItem->edgeProp.id = MEM_ID_PARTITION_BASE + i * MEM_ID_PARTITION_OFFSET + 2;
        partitionItem->edgeProp.name = "partition edgeProp";
#if HAVE_EDGE_PROP
        partitionItem->edgeProp.attr = ATTR_PL_HBM;
#else
        partitionItem->edgeProp.attr = ATTR_HOST_ONLY;
#endif
        partitionItem->edgeProp.unit_size = size * sizeof(int);
        partitionItem->edgeProp.size_attr = SIZE_USER_DEFINE;

        partitionItem->edgeProp.ext_attr.flags = bank[2 * SUB_PARTITION_NUM + cuIndex]; // marked as unused

        he_mem_init(context, &partitionItem->edgeProp);

        partitionItem->tmpProp.id = MEM_ID_PARTITION_BASE + i * MEM_ID_PARTITION_OFFSET + 3;
        partitionItem->tmpProp.name = "partition tmpProp";
        partitionItem->tmpProp.attr = ATTR_PL_HBM;
        partitionItem->tmpProp.unit_size = MAX_VERTICES_IN_ONE_PARTITION * sizeof(int);
        partitionItem->tmpProp.size_attr = SIZE_USER_DEFINE;

        partitionItem->tmpProp.ext_attr.flags = bank[2 * cuIndex + 1];

        he_mem_init(context, &partitionItem->tmpProp);
    }
}

