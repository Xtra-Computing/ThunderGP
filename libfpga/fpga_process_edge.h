#ifndef __FPGA_PROCESS_EDGE_H__
#define __FPGA_PROCESS_EDGE_H__


#include "graph_fpga.h"

#include "fpga_raw_solver.h"

#define BURST_WRITE_SIZE        (64)
#define LOG2_BURST_WRITE_SIZE   (6)
//#define BURST_WRITE_BUFFER_SIZE (BURST_WRITE_SIZE *SIZE_BY_INT)



#define TOTAL_STEAM_READ_NUM    ((BURST_WRITE_SIZE * SIZE_BY_INT / 2)>> LOG2_PE_NUM)

void processEdgeWrite(
    uint_raw                sink_offset,
    uint_raw                sink_end,
    hls::stream<uint_uram>  (&writeArray)[PE_NUM],
    uint16                  *tmpVertexProp
)
{
    uint_raw dstStart = sink_offset;
    uint_raw dstEnd   = sink_end;
    uint_raw vertexNum = dstEnd - dstStart;


    uint16 writeBuffer[BURST_WRITE_SIZE];

    uint16 writeBufferTmp[2];
#pragma HLS ARRAY_PARTITION variable=writeBufferTmp dim=0 complete

    uint_uram writeValue[TOTAL_STEAM_READ_NUM][PE_NUM];
#pragma HLS ARRAY_PARTITION variable=writeValue dim=0 complete

    uint_uram tmpWriteValue[PE_NUM];
#pragma HLS ARRAY_PARTITION variable=tmpWriteValue dim=0 complete
    for (int i = 0; i < PE_NUM; i++)
    {
        tmpWriteValue[i] = 0;
    }

    for (int i = 0; i < ((VERTEX_MAX) >> ( LOG2_BURST_WRITE_SIZE + LOG2_SIZE_BY_INT)); i++)
    {
        for (int sr_idx = 0; sr_idx < TOTAL_STEAM_READ_NUM; sr_idx++)
        {
#pragma HLS PIPELINE II=1
            for (int inner_idx = 0; inner_idx < PE_NUM ; inner_idx++)
            {
#pragma HLS UNROLL
                read_from_stream(writeArray[inner_idx], tmpWriteValue[inner_idx]);
            }
            for (unsigned int inner_idx = 0 ; inner_idx < PE_NUM ; inner_idx ++)
            {
#pragma HLS UNROLL
                writeBufferTmp[0].range(31 + (inner_idx << INT_WIDTH_SHIFT), 0 + (inner_idx << INT_WIDTH_SHIFT))
                    = tmpWriteValue[inner_idx].range(31,  0);
                writeBufferTmp[1].range(31 + (inner_idx << INT_WIDTH_SHIFT), 0 + (inner_idx << INT_WIDTH_SHIFT))
                    = tmpWriteValue[inner_idx].range(63, 32);
            }
            unsigned int burst_index  = sr_idx << 1;
            writeBuffer[burst_index]     = writeBufferTmp[0];
            writeBuffer[burst_index + 1] = writeBufferTmp[1];
        }
        for (int burst_index = 0; burst_index < BURST_WRITE_SIZE; burst_index++)
        {
#pragma HLS PIPELINE II=1
#pragma HLS loop_flatten off
            //DEBUG_PRINTF("%d \n", int (((dstStart >> LOG2_SIZE_BY_INT)) + (i << LOG2_BURST_WRITE_SIZE) + burst_index));

            tmpVertexProp[((dstStart >> LOG2_SIZE_BY_INT)) + (i << LOG2_BURST_WRITE_SIZE) + burst_index ]
                = writeBuffer[burst_index];

        }
    }

    //for (int inner_idx = 0; inner_idx < PE_NUM ; inner_idx++)
    //{
    //    clear_stream(writeArray[inner_idx]);
    //}
}


void dstPropertyProcess(
    int                     index,
    uint_raw                sink_offset,
    uint_raw                sink_end,
    uint_uram               tmpVPropBuffer[(VERTEX_MAX / 2) >> LOG2_PE_NUM],
    //uint_raw                bitmap[BITMAP_SLICE_SIZE][BITMAP_SUB_SIZE],
    hls::stream<int2_token> &buildArray,
    hls::stream<uint_uram>  &writeArray
)
{
#pragma HLS function_instantiate variable=index
#pragma HLS dependence variable=tmpVPropBuffer inter false

    uint_raw dstStart = sink_offset;
    uint_raw dstEnd   = sink_end;
    uint_raw vertexNum = dstEnd - dstStart;


    while (true) {
#pragma HLS PIPELINE II=2
        int2 tmp_data;
        int2_token in_token;
        read_from_stream(buildArray, in_token);
        tmp_data = in_token.data;
        if (in_token.flag == FLAG_SET)
        {
            break;
        }
        uint_raw idx;
        {
#pragma HLS latency min=0 max=0
            uint_raw dstVidx  = tmp_data.x;
            idx = ((dstVidx - dstStart) >> LOG2_PE_NUM ) & ((VERTEX_MAX >> LOG2_PE_NUM) - 1);
        }

        {
#pragma HLS latency min=3 max=3
            uint_uram updated_value = tmpVPropBuffer[(idx >> 1)];
            uint_raw origin_value;

            if (idx & 0x01)
                origin_value = updated_value.range(63, 32);
            else
                origin_value = updated_value.range(31, 0);

            prop_union_t in;
            prop_union_t out;

            in.ui = origin_value;


            out.f = PROP_COMPUTE_STAGE3(in.f, tmp_data.y);

            uint_uram tmp;
            if (idx & 0x01)
            {
                tmp.range(63, 32) = out.ui;
                tmp.range(31,  0) = updated_value.range(31, 0);
            }
            else
            {
                tmp.range(63, 32) = updated_value.range(63, 32);
                tmp.range(31,  0) = out.ui;
            }
            tmpVPropBuffer[(idx >> 1)] = tmp;
        }


    }
    //clear_stream(buildArray);

    /* clear stream */
    //int end_counter = 0;
    //while (true)
    //{
    //    int2 clear_data;

    //    if ( read_from_stream_nb(buildArray, clear_data) == 0)
    //    {
    //        end_counter ++;
    //    }
    //    if (end_counter > 256)
    //    {
    //        break;
    //    }
    //}
    for (int i = 0; i < ((VERTEX_MAX ) >> (LOG2_PE_NUM + 1)); i++)
    {
#pragma HLS PIPELINE II=1
        uint_uram tmp = tmpVPropBuffer[i];
        write_to_stream(writeArray, tmp);
        tmpVPropBuffer[i] = 0;
    }
}


#endif /* __FPGA_PROCESS_EDGE_H__ */
