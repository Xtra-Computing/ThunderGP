#include "host_graph_api.h"
#include "fpga_application.h"

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
    int *tempPropValue   =  (int*)get_host_mem_pointer(MEM_ID_PROP_FOR_DATAPREPARE);
    int *outDeg          =  (int*)get_host_mem_pointer(MEM_ID_OUT_DEG_ORIGIN);
    prop_t *vertexPushinProp = (prop_t*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP);

    int vertexNum = info->vertexNum;
    int alignedVertexNum = get_he_mem(MEM_ID_PUSHIN_PROP)->size/sizeof(int);

    float init_score_float = 1.0f / vertexNum;
    int init_score_int = float2int(init_score_float);


    for (int i = 0; i < vertexNum; i++) {
        tempPropValue[i] = init_score_int;
        if (outDeg[i] > 0)
        {
            vertexPushinProp[i] =  tempPropValue[i] / outDeg[i];
        }
        else
        {
            vertexPushinProp[i]  = 0;
        }
    }

    for (int i = vertexNum; i < alignedVertexNum; i++) {
        vertexPushinProp[i]  = 0;
    }
    return 0;
}