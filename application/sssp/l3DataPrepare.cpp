#include "host_graph_sw.h"
#include "fpga_application.h"

unsigned int dataPrepareGetArg(graphInfo *info)
{
    return 0;
}

int dataPrepareProperty(graphInfo *info)
{
    int *vertexPushinProp =      (int*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP);
    unsigned int *activeVertexNum = (unsigned int *)get_host_mem_pointer(MEM_ID_ACTIVE_VERTEX_NUM);

    int vertexNum = info->vertexNum;

    for (int i = 0; i < (vertexNum / (ALIGN_SIZE ) + 1) * (ALIGN_SIZE ); i++) {
        vertexPushinProp[i]    = MAX_PROP;
    }
    vertexPushinProp[getStartIndex()] = 0x80000001;
    activeVertexNum[0] = 1;
    return 0;
}