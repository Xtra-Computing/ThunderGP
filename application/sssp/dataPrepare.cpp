#include "host_graph_api.h"
#include "fpga_application.h"

unsigned int dataPrepareGetArg(graphInfo *info)
{
    return 0;
}

int dataPrepareProperty(graphInfo *info)
{
    int *vertexPushinProp =      (int*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP);

    int alignedVertexNum = get_he_mem(MEM_ID_PUSHIN_PROP)->size / sizeof(int);

    for (int i = 0; i < alignedVertexNum; i++)
    {
        vertexPushinProp[i]    = MAX_PROP;
    }
    prop_t *edgeProp    = (prop_t*)get_host_mem_pointer(MEM_ID_EDGE_PROP);

    //int edgeNum = info->edgeNum;
    int alignedEdgeNum = get_he_mem(MEM_ID_EDGE_PROP)->size / sizeof(int);

    for (int i = 0; i < alignedEdgeNum; i++)
    {
        edgeProp[i] = i % 16;
    }
    int select_index  = ((double)std::rand()) / ((RAND_MAX + 1u) / info->vertexNum);
    vertexPushinProp[select_index] = 0x80000001;

    return 0;
}