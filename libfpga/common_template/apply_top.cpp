#include <hls_stream.h>
#include "graph_fpga.h"


#include "fpga_global_mem.h"
#include "fpga_apply.h"



extern "C" {
    void  vertexApply(
        uint16        *vertexProp,
        uint16        *tmpVertexProp%THUNDERGP_VERTEXPROP_ID%,
        uint16        *newVertexProp%THUNDERGP_VERTEXPROP_ID%,
#if HAVE_APPLY_OUTDEG
        uint16        *outDegree,
#endif
        int           *outReg,
        unsigned int  vertexNum,
        unsigned int  addrOffset,
        unsigned int  argReg
    )
    {


#pragma HLS INTERFACE m_axi port=tmpVertexProp%THUNDERGP_VERTEXPROP_ID% offset=slave bundle=gmem%THUNDERGP_GMEM_ID% max_read_burst_length=64 num_write_outstanding=4

#pragma HLS INTERFACE s_axilite port=tmpVertexProp%THUNDERGP_VERTEXPROP_ID% bundle=control

#pragma HLS INTERFACE m_axi port=newVertexProp%THUNDERGP_VERTEXPROP_ID% offset=slave bundle=gmem%THUNDERGP_GMEM_ID% max_read_burst_length=64 num_write_outstanding=4

#pragma HLS INTERFACE s_axilite port=newVertexProp%THUNDERGP_VERTEXPROP_ID% bundle=control


#pragma HLS INTERFACE m_axi port=outReg offset=slave bundle=gmem0
#pragma HLS INTERFACE s_axilite port=outReg bundle=control


#pragma HLS INTERFACE m_axi port=vertexProp offset=slave bundle=gmem2 max_read_burst_length=64
#pragma HLS INTERFACE s_axilite port=vertexProp bundle=control

#if HAVE_APPLY_OUTDEG

#pragma HLS INTERFACE m_axi port=outDegree offset=slave bundle=gmem0 max_read_burst_length=64
#pragma HLS INTERFACE s_axilite port=outDegree bundle=control

        hls::stream<burst_raw>      outDegreeStream;
#pragma HLS stream variable=outDegreeStream depth=256
        burstReadLite(addrOffset, vertexNum, outDegree, outDegreeStream);

#endif

#pragma HLS INTERFACE s_axilite port=vertexNum      bundle=control
#pragma HLS INTERFACE s_axilite port=argReg         bundle=control
#pragma HLS INTERFACE s_axilite port=addrOffset     bundle=control
#pragma HLS INTERFACE s_axilite port=return         bundle=control

#pragma HLS DATAFLOW

        hls::stream<burst_raw>      vertexPropStream;
#pragma HLS stream variable=vertexPropStream depth=128

        hls::stream<burst_raw>      tmpVertexPropArray[SUB_PARTITION_NUM];
#pragma HLS stream variable=tmpVertexPropArray depth=2


        hls::stream<burst_raw>      tmpVertexPropStream[SUB_PARTITION_NUM + 1];
#pragma HLS stream variable=tmpVertexPropStream depth=2



        hls::stream<burst_raw>      newVertexPropStream;
#pragma HLS stream variable=newVertexPropStream depth=16

        hls::stream<burst_raw>      newVertexPropArray[SUB_PARTITION_NUM];
#pragma HLS stream variable=newVertexPropArray depth=2

        int loopNum = (vertexNum >> 4) ;



        burstReadLite(addrOffset, vertexNum, vertexProp, vertexPropStream);


        burstReadLite(0, vertexNum, tmpVertexProp%THUNDERGP_VERTEXPROP_ID%, tmpVertexPropArray[%THUNDERGP_VERTEXPROP_ID%]);

        for (int i = 0; i < SUB_PARTITION_NUM; i++)
        {
#pragma HLS UNROLL
            cuMerge(loopNum, tmpVertexPropArray[i], tmpVertexPropStream[i], tmpVertexPropStream[i + 1]);
        }

        applyFunction(
            loopNum,
#if HAVE_APPLY_OUTDEG
            outDegreeStream,
#endif
            vertexPropStream,
            tmpVertexPropStream[SUB_PARTITION_NUM],
            argReg,
            newVertexPropStream,
            outReg
        );

        cuDuplicate(loopNum , newVertexPropStream,
                    newVertexPropArray);


        writeBackLite(vertexNum, newVertexProp%THUNDERGP_VERTEXPROP_ID% + (addrOffset >> 4), newVertexPropArray[%THUNDERGP_VERTEXPROP_ID%]);
    }

}