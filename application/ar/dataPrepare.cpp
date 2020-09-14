#include "host_graph_api.h"
#include "fpga_application.h"


static unsigned int avg_outdegree;

unsigned int dataPrepareGetArg(graphInfo *info)
{
    return avg_outdegree;
}

int dataPrepareProperty(graphInfo *info)
{
    int *outDeg              = (int *)get_host_mem_pointer(MEM_ID_OUT_DEG_ORIGIN);
    prop_t *vertexPushinProp = (prop_t*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP);

    int vertexNum = info->vertexNum;
    int alignedVertexNum = get_he_mem(MEM_ID_PUSHIN_PROP)->size/sizeof(int);

    unsigned int total_outdegree = 0;

    for (int i = 0; i < vertexNum; i++)
    {
        vertexPushinProp[i]  = 0;
        total_outdegree +=  outDeg[i];
    }
    avg_outdegree = (unsigned int)(((double)total_outdegree) / vertexNum);

    for (int i = vertexNum; i < alignedVertexNum; i++) {
        vertexPushinProp[i]  = 0;
    }
    return 0;
}