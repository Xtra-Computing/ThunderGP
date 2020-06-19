#include "host_graph_sw.h"

#include "he_mem_config.h"

#define EDEG_MEMORY_SIZE        ((edgeNum + (ALIGN_SIZE * 4) * blkNum) * 1)
#define VERTEX_MEMORY_SIZE      (((vertexNum - 1)/VERTEX_MAX + 1) * VERTEX_MAX)

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


void process_mem_init(cl_context &context)
{
    int *vertexProp =       (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE_CACHED);
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        gs_mem_init(context, getGatherScatter(i), i, vertexProp);
    }
}

void partition_mem_init(cl_context &context, int blkIndex, int size, int cuIndex)
{
    int i = blkIndex;
    subPartitionDescriptor *partitionItem  = getSubPartition(i);
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