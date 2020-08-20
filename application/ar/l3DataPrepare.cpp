#include "host_graph_sw.h"
#include "fpga_application.h"

#define INT2FLOAT                   (pow(2,30))

int float2int(float a) {
    return (int)(a * INT2FLOAT);
}

float int2float(int a) {
    return ((float)a / INT2FLOAT);
}

static unsigned int avg_outdegree;

unsigned int dataPrepareGetArg(graphInfo *info)
{
    return avg_outdegree;
}

int dataPrepareProperty(graphInfo *info)
{
    int *outDeg =           (int*)get_host_mem_pointer(MEM_ID_OUT_DEG_ORIGIN);
    int *vertexPushinProp =      (int*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP);

    int vertexNum = info->vertexNum;

    unsigned int total_outdegree = 0;


    for (int i = 0; i < vertexNum; i++) {
        vertexPushinProp[i]  = 0;
        total_outdegree +=  outDeg[i];
    }
    avg_outdegree = (unsigned int)(((double)total_outdegree)/vertexNum);

    for (int i = vertexNum; i < (vertexNum / (ALIGN_SIZE ) + 1) * (ALIGN_SIZE ); i++) {
        vertexPushinProp[i]  = 0;
    }
    return 0;
}