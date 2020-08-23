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
    vertexPushinProp[getStartIndex()] = 0x80000001;
    return 0;
}