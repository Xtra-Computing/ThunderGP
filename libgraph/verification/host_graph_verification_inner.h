#ifndef __HOST_GRAPH_SW_VERIFICATION_INNER_H__
#define __HOST_GRAPH_SW_VERIFICATION_INNER_H__

#include "host_graph_sw.h"


//#define SW_DEBUG

#define DEBUG_DUMP_VERTEX_SIZE          (1024)

#define DATA_DUMP                       0//(i == 112)

//#define PROBE_VERTEX                    (112)


void partitionGatherScatterCModel(
    cl_context              &context,
    cl_device_id            &device,
    int                     superStep,
    int                     cuIndex,
    subPartitionDescriptor  *subPartitions
);

void partitionApplyCModel(
    cl_context              &context,
    cl_device_id            &device,
    int                     superStep,
    int                     partId,
    unsigned int            applyArg
);


#endif /* __HOST_GRAPH_SW_VERIFICATION_H__ */
