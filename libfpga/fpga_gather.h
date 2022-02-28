#ifndef __FPGA_GATHER_H__
#define __FPGA_GATHER_H__

#include "graph_fpga.h"

#include "fpga_decoder.h"

void shuffleDispatcher(
    int                         i,
    uchar                       &opcode,
    filter_type                 &filter,
    hls::stream<filter_type>    &toFilterArray

)
{
    if (opcode | (filter.end == 1)) {
        write_to_stream(toFilterArray, filter);
    }
}


void shuffleEntry (
    int                         i,
    hls::stream<edge_tuples_t>  &edgeTuplesArray,
    hls::stream<edge_tuples_t>  &toNextShuffle,
    hls::stream<filter_type>    &toFilterArray,
    filter_type                 &filter,
    edge_tuples_t               &tuples,
    uchar                       &opcode,
    shuffled_type               &shuff_ifo)
{
#pragma HLS function_instantiate variable=i



    unsigned char unitFinishFlag[EDGE_NUM];
#pragma HLS ARRAY_PARTITION variable=unitFinishFlag dim=0 complete

    for (int j = 0; j < EDGE_NUM; j++) {
#pragma HLS UNROLL
        unitFinishFlag[j] = 0;
    }
    unsigned char end_flag_shift;


    while (true)
    {
#pragma HLS PIPELINE II=1
        read_from_stream(edgeTuplesArray, tuples);
        write_to_stream(toNextShuffle, tuples);

        uchar valid_r[8];
#pragma HLS ARRAY_PARTITION variable=valid_r dim=0 complete

        uchar idx[8];
#pragma HLS ARRAY_PARTITION variable=idx dim=0 complete
        // each collect engine do their work
        //int16 data_r;
        for (int j = 0; j < 8; j ++)
        {
#pragma HLS UNROLL
            valid_r[j] = 0;
            idx[j] = 0;
        }
        end_flag_shift = tuples.flag;

        //data_r = tuples.data;

        valid_r[0] = ((tuples.data[0].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[0].y);
        valid_r[1] = ((tuples.data[1].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[1].y);
        valid_r[2] = ((tuples.data[2].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[2].y);
        valid_r[3] = ((tuples.data[3].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[3].y);
        valid_r[4] = ((tuples.data[4].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[4].y);
        valid_r[5] = ((tuples.data[5].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[5].y);
        valid_r[6] = ((tuples.data[6].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[6].y);
        valid_r[7] = ((tuples.data[7].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[7].y);


        opcode = valid_r[0]
                 + (valid_r[1] << 1)
                 + (valid_r[2] << 2)
                 + (valid_r[3] << 3)
                 + (valid_r[4] << 4)
                 + (valid_r[5] << 5)
                 + (valid_r[6] << 6)
                 + (valid_r[7] << 7);

        shuff_ifo  = shuffleDecoder(opcode);

        filter.num = shuff_ifo.num;


        filter.idx = shuff_ifo.idx;


        for (int j = 0; j < EDGE_NUM; j ++) {
#pragma HLS UNROLL
            filter.data[j] = tuples.data[j];  //data_r_uint2[idx_t];
        }
        // idx[0] =  shuff_ifo.idx & 0x7;
        // idx[1] = (shuff_ifo.idx >> 3) & 0x7;
        // idx[2] = (shuff_ifo.idx >> 6) & 0x7;
        // idx[3] = (shuff_ifo.idx >> 9) & 0x7;
        // idx[4] = (shuff_ifo.idx >> 12) & 0x7;
        // idx[5] = (shuff_ifo.idx >> 15) & 0x7;
        // idx[6] = (shuff_ifo.idx >> 18) & 0x7;
        // idx[7] = (shuff_ifo.idx >> 21) & 0x7;

        filter.end = 0;

        if ((end_flag_shift) != 0)
        {
            filter.end = 1;
            filter.num = 1;
        }
        else
        {
            filter.end = 0;
        }

        //shuffleDispatcher(i, opcode, filter, toFilterArray);

        if (opcode | (filter.end == 1)) {
            write_to_stream(toFilterArray, filter);
        }

        if (filter.end != 0)
        {
            break;
        }
    }
    return;
}

void theLastShuffleEntry (
    int                         i,
    hls::stream<edge_tuples_t>  &edgeTuplesArray,
    hls::stream<filter_type>    &toFilterArray,
    filter_type                 &filter,
    edge_tuples_t               &tuples,
    uchar                       &opcode,
    shuffled_type               &shuff_ifo)
{
#pragma HLS function_instantiate variable=i



    unsigned char unitFinishFlag[EDGE_NUM];
#pragma HLS ARRAY_PARTITION variable=unitFinishFlag dim=0 complete

    for (int j = 0; j < EDGE_NUM; j++) {
#pragma HLS UNROLL
        unitFinishFlag[j] = 0;
    }
    unsigned char end_flag_shift;


    while (true)
    {
#pragma HLS PIPELINE II=1
        read_from_stream(edgeTuplesArray, tuples);


        uchar valid_r[8];
#pragma HLS ARRAY_PARTITION variable=valid_r dim=0 complete

        uchar idx[8];
#pragma HLS ARRAY_PARTITION variable=idx dim=0 complete
        // each collect engine do their work
        //int16 data_r;
        for (int j = 0; j < 8; j ++)
        {
#pragma HLS UNROLL
            valid_r[j] = 0;
            idx[j] = 0;
        }
        end_flag_shift = tuples.flag;

        //data_r = tuples.data;

        valid_r[0] = ((tuples.data[0].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[0].y);
        valid_r[1] = ((tuples.data[1].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[1].y);
        valid_r[2] = ((tuples.data[2].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[2].y);
        valid_r[3] = ((tuples.data[3].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[3].y);
        valid_r[4] = ((tuples.data[4].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[4].y);
        valid_r[5] = ((tuples.data[5].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[5].y);
        valid_r[6] = ((tuples.data[6].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[6].y);
        valid_r[7] = ((tuples.data[7].x & HASH_MASK) == i) && IS_ACTIVE_VERTEX(tuples.data[7].y);


        opcode = valid_r[0]
                 + (valid_r[1] << 1)
                 + (valid_r[2] << 2)
                 + (valid_r[3] << 3)
                 + (valid_r[4] << 4)
                 + (valid_r[5] << 5)
                 + (valid_r[6] << 6)
                 + (valid_r[7] << 7);

        shuff_ifo  = shuffleDecoder(opcode);

        filter.num = shuff_ifo.num;

        filter.idx = shuff_ifo.idx;


        for (int j = 0; j < EDGE_NUM; j ++) {
#pragma HLS UNROLL
            filter.data[j] = tuples.data[j];  //data_r_uint2[idx_t];
        }
        // idx[0] =  shuff_ifo.idx & 0x7;
        // idx[1] = (shuff_ifo.idx >> 3) & 0x7;
        // idx[2] = (shuff_ifo.idx >> 6) & 0x7;
        // idx[3] = (shuff_ifo.idx >> 9) & 0x7;
        // idx[4] = (shuff_ifo.idx >> 12) & 0x7;
        // idx[5] = (shuff_ifo.idx >> 15) & 0x7;
        // idx[6] = (shuff_ifo.idx >> 18) & 0x7;
        // idx[7] = (shuff_ifo.idx >> 21) & 0x7;

        filter.end = 0;

        if ((end_flag_shift) != 0)
        {
            filter.end = 1;
            filter.num = 1;
        }
        else
        {
            filter.end = 0;
        }

        //shuffleDispatcher(i, opcode, filter, toFilterArray);

        if (opcode | (filter.end == 1)) {
            write_to_stream(toFilterArray, filter);
        }

        if (filter.end != 0)
        {
            break;
        }
    }
    return;
}

#endif /* __FPGA_GATHER_H__ */
