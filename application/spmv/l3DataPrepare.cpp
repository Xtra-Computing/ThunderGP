#include "host_graph_sw.h"


unsigned int dataPrepareGetArg(graphInfo *info)
{
    return 0;
}

int dataPrepareProperty(graphInfo *info)
{
    int *vertexProp =       (int*)get_host_mem_pointer(MEM_ID_VERTEX_PROP);
    int *vertexScore =      (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE);
    unsigned int *activeVertexNum = (unsigned int *)get_host_mem_pointer(MEM_ID_ACTIVE_VERTEX_NUM);


    int vertexNum = info->vertexNum;
    int edgeNum   = info->edgeNum;

    for (int i = 0; i < (vertexNum / (ALIGN_SIZE ) + 1) * (ALIGN_SIZE ); i++) {
        vertexScore[i]    = MAX_PROP;
    }
    vertexScore[source] = 0x80000001;
    activeVertexNum[0] = 1;
}