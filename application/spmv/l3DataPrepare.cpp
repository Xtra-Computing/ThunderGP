#include "host_graph_sw.h"


unsigned int dataPrepareGetArg(graphInfo *info)
{
    return 0;
}

int dataPrepareProperty(graphInfo *info)
{
    int *vertexScore =      (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE);

    int vertexNum = info->vertexNum;

    for (int i = 0; i < (vertexNum / (ALIGN_SIZE ) + 1) * (ALIGN_SIZE ); i++) {
        vertexScore[i]    = i;
    }
    return 0;
}