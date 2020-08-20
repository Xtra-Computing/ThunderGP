#include "host_graph_sw.h"


unsigned int dataPrepareGetArg(graphInfo *info)
{
    return 0;
}

int dataPrepareProperty(graphInfo *info)
{
    int *vertexPushinProp =      (int*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP);

    int vertexNum = info->vertexNum;

    for (int i = 0; i < (vertexNum / (ALIGN_SIZE ) + 1) * (ALIGN_SIZE ); i++) {
        vertexPushinProp[i]    = i;
    }
    return 0;
}