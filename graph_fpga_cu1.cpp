#include <hls_stream.h>
#include <string.h>
#include "graph_fpga.h"

#include "fpga_burst_read.h"
#include "fpga_slice.h"
#include "fpga_gather.h"
#include "fpga_filter.h"
#include "fpga_process_edge.h"
#include "fpga_cache.h"



//avoid bad timing when use subdevice provided by xilinx

extern "C" {
    void  readEdgesCU1(
        uint16          *edgeScoreMap,
        uint16          *vertexScore,
        uint16          *edges,
        uint16          *tmpVertexProp,
        int             edge_end,
        int             sink_offset,
        int             sink_end
    )
    {
#include "fpga_gs_top.h"
    }

} // extern C

