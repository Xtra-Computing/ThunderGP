#include "host_graph_sw.h"
#include "fpga_application.h"

unsigned int dataPrepareGetArg(graphInfo *info)
{
    return 0;
}

int dataPrepareProperty(graphInfo *info)
{
    int *vertexPushinProp =      (int*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP);

    int vertexNum = info->vertexNum;
    int alignedVertexNum = get_he_mem(MEM_ID_PUSHIN_PROP)->size;

    for (int i = 0; i < alignedVertexNum; i++)
    {
        vertexPushinProp[i]    = MAX_PROP;
    }
    prop_t *edgeProp    = (prop_t*)get_host_mem_pointer(MEM_ID_EDGE_PROP);

    //int edgeNum = info->edgeNum;
    int alignedEdgeNum = get_he_mem(MEM_ID_EDGE_PROP)->size;

    for (int i = 0; i < alignedEdgeNum; i++)
    {
        edgeProp = i;
    }

    vertexPushinProp[getStartIndex()] = 0x80000001;

    return 0;
}