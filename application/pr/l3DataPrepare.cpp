#include "host_graph_sw.h"
#include "l2.h"

#define INT2FLOAT                   (pow(2,30))

int float2int(float a) {
    return (int)(a * INT2FLOAT);
}

float int2float(int a) {
    return ((float)a / INT2FLOAT);
}

unsigned int dataPrepareGetArg(graphInfo *info)
{
    return float2int((1.0f - kDamp) / info->vertexNum);
}

int dataPrepareProperty(graphInfo *info)
{
    int *vertexProp =       (int*)get_host_mem_pointer(MEM_ID_VERTEX_PROP);
    int *outDeg =           (int*)get_host_mem_pointer(MEM_ID_OUT_DEG_ORIGIN);
    int *vertexScore =      (int*)get_host_mem_pointer(MEM_ID_VERTEX_SCORE);

    int vertexNum = info->vertexNum;

    float init_score_float = 1.0f / vertexNum;
    int init_score_int = float2int(init_score_float);

    
    for (int i = 0; i < vertexNum; i++) {
        vertexProp[i] = init_score_int;
        if (outDeg[i] > 0)
        {
            vertexScore[i] =  vertexProp[i] / outDeg[i];
        }
        else
        {
            vertexScore[i]  = 0;
        }
    }
    
    for (int i = vertexNum; i < (vertexNum / (ALIGN_SIZE ) + 1) * (ALIGN_SIZE ); i++) {
        vertexScore[i]  = 0;
    }
    return 0;
}