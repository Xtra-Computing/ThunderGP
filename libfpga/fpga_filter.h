#ifndef __FPGA_FILTER_H__
#define __FPGA_FILTER_H__

#include "graph_fpga.h"


void tupleFilter(
    filter_type                 &filter,
    uint_raw                    &filter_num,
    hls::stream<filter_type>    &toFilterItem,
    hls::stream<int2_token>     &buildArray
)
{
#pragma HLS function_instantiate variable=filter_num

    uint_raw filter_end;
    while (true) {
#pragma HLS PIPELINE II=1
#pragma HLS dependence variable=filter inter false
#pragma HLS dependence variable=buildArray inter false
#pragma HLS dependence variable=filter_end inter false

        read_from_stream(toFilterItem, filter);
        filter_end = filter.end;
        filter_num = filter.num;

        uchar idx[8];
#pragma HLS ARRAY_PARTITION variable=idx dim=0 complete
        
        idx[0] = filter.idx.range(2,0);
        idx[1] = filter.idx.range(5,3);
        idx[2] = filter.idx.range(8,6);
        idx[3] = filter.idx.range(11,9);
        idx[4] = filter.idx.range(14,12);
        idx[5] = filter.idx.range(17,15);
        idx[6] = filter.idx.range(20,18);
        idx[7] = filter.idx.range(23,21);

        for (int j = 0; j < filter_num; j ++) {
#pragma HLS PIPELINE II=1 rewind
#pragma HLS unroll factor=1
            {   
                //uchar idx = (filter.idx >> (3 * j)) & 0x7;
                int2_token token;
                token.data = filter.data[idx[j]];
                token.flag = (j == (filter_num - 1 ))? filter.end: 0;
                write_to_stream(buildArray, token);
            }
        }
        if (filter_end)
        {
            break;
        }

    }
    //clear_stream(toFilterItem);
    return;
}


#endif /* __FPGA_FILTER_H__ */

