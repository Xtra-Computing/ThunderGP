#ifndef __FPGA_EDGE_PROP_H__
#define __FPGA_EDGE_PROP_H__

#include "graph_fpga.h"

#include "fpga_application.h"

void halfEdgeProp(
    hls::stream<burst_token>    &input,
    hls::stream<burst_half>     &output
)
{
    while (true)
    {
#pragma HLS PIPELINE II=2
        burst_token  tmp;
        read_from_stream(input, tmp);
        burst_half out1 = tmp.data.range(255,   0);
        burst_half out2 = tmp.data.range(511, 256);
        write_to_stream(output, out1);
        write_to_stream(output, out2);
        if (tmp.flag == FLAG_SET)
        {
            break;
        }
    }
}


void edgePropCouple (
    hls::stream<edge_tuples_t>   &input,
    hls::stream<burst_half>      &edgeProp,
    hls::stream<edge_tuples_t>   &output)
{

    while (true)
    {
#pragma HLS PIPELINE II=1
        edge_tuples_t  tuples;
        edge_tuples_t  out;
        read_from_stream(input, tuples);
        burst_half     prop;
        read_from_stream(edgeProp, prop);

        for (int i = 0; i < EDGE_NUM; i++)
        {
#pragma HLS UNROLL
            out.data[i].x = tuples.data[i].x;
            out.data[i].y =  PROP_COMPUTE_STAGE1(tuples.data[i].y, prop.range(31 + 32 * i, 0 + 32 * i));
            out.flag = tuples.flag;
        }
        write_to_stream(output, out);
        if (tuples.flag == FLAG_SET)
        {
            break;
        }
    }
    clear_stream(input);
    clear_stream(edgeProp);
}

void propProcess( hls::stream<burst_token>     &propInput,
                  hls::stream<edge_tuples_t>   &tupleInput,
                  hls::stream<edge_tuples_t>   &tupleOuput
                )
{
#pragma HLS DATAFLOW

    hls::stream<burst_half>      halfPropStream;
#pragma HLS stream variable=halfPropStream depth=2

    halfEdgeProp(propInput, halfPropStream);
    edgePropCouple(tupleInput, halfPropStream, tupleOuput);

}


void propProcessSelf( hls::stream<edge_tuples_t>   &tupleInput,
                      hls::stream<edge_tuples_t>   &tupleOuput
                    )

{
    while (true)
    {
#pragma HLS PIPELINE II=1
        edge_tuples_t  in;
        edge_tuples_t  out;
        read_from_stream(tupleInput, in);

        for (int i = 0; i < EDGE_NUM; i++)
        {
#pragma HLS UNROLL
            out.data[i].x = in.data[i].x;
            out.data[i].y = PROP_COMPUTE_STAGE0(in.data[i].y);
            out.flag = in.flag;
        }
        write_to_stream(tupleOuput, out);
        if (in.flag == FLAG_SET)
        {
            break;
        }
    }
    clear_stream(tupleInput);
}

#endif /* __FPGA_EDGE_PROP_H__ */
