#include "host_graph_api.h"
#include "fpga_application.h"

#include <cstdlib>
#include <iostream>
#include <ctime>


unsigned int dataPrepareGetArg(graphInfo *info)
{
    return 0;
}

int dataPrepareProperty(graphInfo *info)
{
    std::srand(std::time(nullptr));
    int *vertexPushinProp =      (int*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP);

    int alignedVertexNum = get_he_mem(MEM_ID_PUSHIN_PROP)->size/sizeof(int);

    for (int i = 0; i < alignedVertexNum; i++)
    {
        vertexPushinProp[i]    = 0;
    }
    for (int i = 0; i < 32; i++)
    {
        int select_index  = ((double)std::rand())/((RAND_MAX + 1u)/info->vertexNum);
        vertexPushinProp[select_index] = 1 << i;
    }
    return 0;
}
