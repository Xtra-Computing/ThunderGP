#include "host_graph_api.h"


unsigned int dataPrepareGetArg(graphInfo *info)
{
	return 0;
}

int dataPrepareProperty(graphInfo *info)
{
	prop_t *vertexPushinProp =      (prop_t*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP);

	int alignedVertexNum = get_he_mem(MEM_ID_PUSHIN_PROP)->size/sizeof(int);

	for (int i = 0; i < alignedVertexNum; i++) {
		vertexPushinProp[i]    = i;
	}
	 prop_t *edgeProp    = (prop_t*)get_host_mem_pointer(MEM_ID_EDGE_PROP);

	//int edgeNum = info->edgeNum;
	int alignedEdgeNum = get_he_mem(MEM_ID_EDGE_PROP)->size/sizeof(int);

	for (int i = 0; i < alignedEdgeNum; i++)
	{
		edgeProp[i] = i;
	}


	return 0;
}