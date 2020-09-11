#include <hls_stream.h>
#include "graph_fpga.h"


#include "fpga_global_mem.h"
#include "fpga_apply.h"

union type_cov {
    float         f;
    prop_t        i;
    unsigned int  ui;
};

extern "C" {
    void  vertexApply(
        uint16        *vertexProp,
#pragma THUNDERGP MSLR_INTERFACE_ARG tmpVertexProp
        uint16        *tmpVertexProp#%d#,
#pragma THUNDERGP MSLR_INTERFACE_ARG newVertexProp
        uint16        *newVertexProp#%d#,
#pragma THUNDERGP USER_APPLY_ARG
        //mark for auto generation
        unsigned int  vertexNum,
        unsigned int  addrOffset
    )
    {
#pragma HLS DATAFLOW

#pragma THUNDERGP MSLR_INTERFACE_ATTR tmpVertexProp

#pragma THUNDERGP MSLR_INTERFACE_ATTR newVertexProp


#pragma HLS INTERFACE m_axi port=vertexProp offset=slave bundle=gmem5 max_read_burst_length=64
#pragma HLS INTERFACE s_axilite port=vertexProp bundle=control

        hls::stream<burst_raw>      vertexPropStream;
#pragma HLS stream variable=vertexPropStream depth=128
        burstReadLite(addrOffset, vertexNum, vertexProp, vertexPropStream);


#pragma THUNDERGP MSLR_INTERFACE_INSTANCE tmpVertexProp
#pragma HLS INTERFACE m_axi port=tmpVertexProp#%d# offset=slave bundle=gmem#%d# max_read_burst_length=64 num_write_outstanding=4

#pragma THUNDERGP MSLR_INTERFACE_INSTANCE tmpVertexProp
#pragma HLS INTERFACE s_axilite port=tmpVertexProp#%d# bundle=control

#pragma THUNDERGP MSLR_INTERFACE_INSTANCE newVertexProp
#pragma HLS INTERFACE m_axi port=newVertexProp#%d# offset=slave bundle=gmem#%d# max_read_burst_length=64 num_write_outstanding=4

#pragma THUNDERGP MSLR_INTERFACE_INSTANCE newVertexProp
#pragma HLS INTERFACE s_axilite port=newVertexProp#%d# bundle=control

#pragma THUNDERGP USER_APPLY_STREAM_ATTR
        //mark for auto generation

#pragma THUNDERGP USER_APPLY_SCALAR_ATTR
        //mark for auto generation

#pragma HLS INTERFACE s_axilite port=vertexNum      bundle=control
#pragma HLS INTERFACE s_axilite port=addrOffset     bundle=control
#pragma HLS INTERFACE s_axilite port=return         bundle=control

        /* tmpVertexProp start*/
        hls::stream<burst_raw>      tmpVertexPropArray[SUB_PARTITION_NUM];
#pragma HLS stream variable=tmpVertexPropArray depth=2

#pragma THUNDERGP MSLR_INTERFACE_INSTANCE tmpVertexProp
        burstReadLite(0, vertexNum, tmpVertexProp#%d#, tmpVertexPropArray[#%d#]);

        hls::stream<burst_raw>      tmpVertexPropStream[SUB_PARTITION_NUM + 1];
#pragma HLS stream variable=tmpVertexPropStream depth=2

        int loopNum = (vertexNum >> 4) ;

        for (int i = 0; i < SUB_PARTITION_NUM; i++)
        {
#pragma HLS UNROLL
            cuMerge(loopNum, tmpVertexPropArray[i], tmpVertexPropStream[i], tmpVertexPropStream[i + 1]);
        }
        /* tmpVertexProp end */

        hls::stream<burst_raw>      newVertexPropStream;
#pragma HLS stream variable=newVertexPropStream depth=16




        for (int loopCount = 0; loopCount < loopNum; loopCount ++)
        {

#pragma HLS PIPELINE II=1
            /* input  */
            burst_raw vertexProp;
            burst_raw tmpVertexProp;

            read_from_stream(vertexPropStream, vertexProp);
            read_from_stream(tmpVertexPropStream[SUB_PARTITION_NUM], tmpVertexProp);

#pragma THUNDERGP USER_APPLY_READ_FROM_STREAM
            //mark for auto generation

            /* output */
            burst_raw newVertexProp;

core_loop: for (int i = 0; i < BURST_ALL_BITS / INT_WIDTH; i++)
            {
#pragma HLS UNROLL
                prop_t  wProp;
                prop_t tProp     = tmpVertexProp.range((i + 1) * INT_WIDTH - 1, i * INT_WIDTH );
                prop_t uProp     = vertexProp.range(   (i + 1) * INT_WIDTH - 1, i * INT_WIDTH );

#pragma THUNDERGP USER_APPLY_COV_FOR_CAL
                //mark for auto generation

#pragma THUNDERGP USER_APPLY_CAL
                //mark for auto generation

                /*
                prop_t  wProp    = applyFunc( tProp, uProp, out_deg, tmpInfoArray[i],  argReg);
                */

#pragma THUNDERGP USER_APPLY_COV_FOR_WRITE
                //mark for auto generation
                newVertexProp.range((i + 1) * INT_WIDTH - 1, i * INT_WIDTH ) = wProp;

            }
#pragma THUNDERGP USER_APPLY_WRITE_TO_STREAM
            //mark for auto generation
            write_to_stream(newVertexPropStream, newVertexProp);
        }


        hls::stream<burst_raw>      newVertexPropArray[SUB_PARTITION_NUM];
#pragma HLS stream variable=newVertexPropArray depth=2


        cuDuplicate(loopNum , newVertexPropStream,
                    newVertexPropArray);

#pragma THUNDERGP MSLR_INTERFACE_INSTANCE newVertexProp
        writeBackLite(vertexNum, newVertexProp#%d# + (addrOffset >> 4), newVertexPropArray[#%d#]);

#pragma THUNDERGP USER_APPLY_WRITE
        //mark for auto generation

    }

}
