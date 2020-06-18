#include <hls_stream.h>
#include <string.h>
#include "graph_fpga.h"

#include "fpga_global_mem.h"
#include "fpga_slice.h"
#include "fpga_gather.h"
#include "fpga_filter.h"
#include "fpga_process_edge.h"
#include "fpga_cache.h"
#include "fpga_edge_prop.h"




extern "C" {
    void  readEdgesCU4(
        uint16          *edgeScoreMap,
        uint16          *vertexScore,
        uint16          *edges,
        uint16          *tmpVertexProp,
#if HAVE_EDGE_PROP
        uint16          *edgeProp,
#endif
        int             edge_end,
        int             sink_offset,
        int             sink_end
    )
    {
#include "fpga_gs_top.h"
    }

} // extern C

