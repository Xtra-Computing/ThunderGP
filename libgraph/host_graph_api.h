#ifndef __HOST_GRAPH_API_H__
#define __HOST_GRAPH_API_H__

#include <string>


#include "common.h"
#include "global_config.h"

typedef struct
{
    int vertexNum;
    int compressedVertexNum;
    int edgeNum;
    int blkNum;
} graphInfo;

/* misc */
unsigned int dataPrepareGetArg(graphInfo *info);
int dataPrepareProperty(graphInfo *info);
double getCurrentTimestamp(void);
void reTransferProp(graphInfo *info);

/* host api -- dataflow */
int acceleratorInit(const char * name, char *file_name);
int acceleratorDataLoad(const std::string &gName, const std::string &mode, graphInfo *info);
int acceleratorDataPreprocess(graphInfo *info);
int acceleratorSuperStep(int superStep, graphInfo *info);
int acceleratorDeinit(void);

/* host api -- query */
void* acceleratorQueryRegister(void);
prop_t* acceleratorQueryProperty(int step);

#include "host_graph_csv.hpp"

#endif /* __HOST_GRAPH_API_H__ */
