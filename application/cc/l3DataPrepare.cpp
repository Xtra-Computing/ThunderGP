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
        vertexPushinProp[i]    = 0;
    }
    for (int i = 0; i < 32; i++)
    {
        vertexPushinProp[getStartIndex() + i * 10] = 1 << i;
    }
    return 0;
}
