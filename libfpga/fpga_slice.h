#ifndef __FPGA_SLICE_H__
#define __FPGA_SLICE_H__

#include "graph_fpga.h"

template <typename T>
void  sliceStream(hls::stream<T>       &input,
                  hls::stream<T>       &output)
{
#pragma HLS function_instantiate variable=input
    while (true)
    {
#pragma HLS PIPELINE II=1
        T  unit;
        read_from_stream(input, unit);
        write_to_stream(output, unit);
        if (unit.flag == FLAG_SET)
        {
            break;
        }
    }
}



template <typename T>
void  duplicateStream2(hls::stream<T>       &input,
                       hls::stream<T>       &output1,
                       hls::stream<T>       &output2)
{
#pragma HLS function_instantiate variable=input
    while (true)
    {
#pragma HLS PIPELINE II=1
        T  unit;
        read_from_stream(input, unit);
        write_to_stream(output1, unit);
        write_to_stream(output2, unit);
        if (unit.range(31, 0) == ENDFLAG)
        {
            break;
        }
    }
}



template <typename T>
void  duplicateStream4(hls::stream<T>       &input,
                       hls::stream<T>       &output1,
                       hls::stream<T>       &output2,
                       hls::stream<T>       &output3,
                       hls::stream<T>       &output4)
{
#pragma HLS function_instantiate variable=input
    while (true)
    {
#pragma HLS PIPELINE II=1
        T  unit;
        read_from_stream(input, unit);
        write_to_stream(output1, unit);
        write_to_stream(output2, unit);
        write_to_stream(output3, unit);
        write_to_stream(output4, unit);
        if (unit.flag == FLAG_SET)
        {
            break;
        }
    }
}

template <typename T>
void  duplicateStream4WithClear(hls::stream<T>       &input,
                                hls::stream<T>       &output1,
                                hls::stream<T>       &output2,
                                hls::stream<T>       &output3,
                                hls::stream<T>       &output4)
{
#pragma HLS function_instantiate variable=input
    while (true)
    {
#pragma HLS PIPELINE II=1
        T  unit;
        read_from_stream(input, unit);
        write_to_stream(output1, unit);
        write_to_stream(output2, unit);
        write_to_stream(output3, unit);
        write_to_stream(output4, unit);
        if (unit.flag == FLAG_SET)
        {
            break;
        }
    }
}



void processEdgesBuildSlice(hls::stream<int2_token>  &in , hls::stream<int2_token> &out)
{
#pragma HLS function_instantiate variable=in
    while (true)
    {
        int2_token tmp_data;
        read_from_stream(in, tmp_data);
        write_to_stream(out, tmp_data);
        if (tmp_data.flag == FLAG_SET)
        {
            break;
        }
    }
}



void processEdgesSlice(hls::stream<uint_uram>  &input, hls::stream<uint_uram>  &output)
{
#pragma HLS function_instantiate variable=input
    for (int i = 0; i < ((MAX_VERTICES_IN_ONE_PARTITION ) >> (LOG2_PE_NUM + 1)); i++)
    {
#pragma HLS PIPELINE II=1
        uint_uram tmp;
        read_from_stream(input, tmp);
        write_to_stream(output, tmp);
    }
}


void filterSlice(
    hls::stream<filter_type>    &input,
    hls::stream<filter_type>    &output
)
{
#pragma HLS function_instantiate variable=input

    while (true) {
        filter_type tmp;

        read_from_stream(input, tmp);

        write_to_stream(output, tmp);
        if (tmp.end)
        {
            break;
        }
    }
}



#endif /* __FPGA_SLICE_H__ */
