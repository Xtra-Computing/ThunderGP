#include <hls_stream.h>
#include "graph_fpga.h"


#include "fpga_global_mem.h"
#include "fpga_apply.h"



extern "C" {
    void  vertexApply(
        uint16        *vertexProp,
#pragma THUNDERGP MSLR_INTERFACE_ARG tmpVertexProp
        uint16        *tmpVertexProp#%d#,
#pragma THUNDERGP MSLR_INTERFACE_ARG newVertexProp
        uint16        *newVertexProp#%d#,
#if HAVE_APPLY_OUTDEG
        uint16        *outDegree,
#endif
        int           *outReg,
        unsigned int  vertexNum,
        unsigned int  addrOffset,
        unsigned int  argReg
    )
    {


#pragma THUNDERGP MSLR_INTERFACE_ATTR tmpVertexProp 

#pragma THUNDERGP MSLR_INTERFACE_ATTR newVertexProp

#pragma THUNDERGP MSLR_INTERFACE_INSTANCE tmpVertexProp
#pragma HLS INTERFACE m_axi port=tmpVertexProp#%d# offset=slave bundle=gmem#%d# max_read_burst_length=64 num_write_outstanding=4

#pragma THUNDERGP MSLR_INTERFACE_INSTANCE tmpVertexProp
#pragma HLS INTERFACE s_axilite port=tmpVertexProp#%d# bundle=control

#pragma THUNDERGP MSLR_INTERFACE_INSTANCE newVertexProp
#pragma HLS INTERFACE m_axi port=newVertexProp#%d# offset=slave bundle=gmem#%d# max_read_burst_length=64 num_write_outstanding=4

#pragma THUNDERGP MSLR_INTERFACE_INSTANCE newVertexProp
#pragma HLS INTERFACE s_axilite port=newVertexProp#%d# bundle=control


#pragma HLS INTERFACE m_axi port=outReg offset=slave bundle=gmem5
#pragma HLS INTERFACE s_axilite port=outReg bundle=control


#pragma HLS INTERFACE m_axi port=vertexProp offset=slave bundle=gmem6 max_read_burst_length=64
#pragma HLS INTERFACE s_axilite port=vertexProp bundle=control

#if HAVE_APPLY_OUTDEG

#pragma HLS INTERFACE m_axi port=outDegree offset=slave bundle=gmem7 max_read_burst_length=64
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


#pragma THUNDERGP MSLR_INTERFACE_INSTANCE tmpVertexProp
        burstReadLite(0, vertexNum, tmpVertexProp#%d#, tmpVertexPropArray[#%d#]);

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


#pragma THUNDERGP MSLR_INTERFACE_INSTANCE newVertexProp
        writeBackLite(vertexNum, newVertexProp#%d# + (addrOffset >> 4), newVertexPropArray[#%d#]);

    }

}
