#ifndef __FPGA_PROCESS_EDGE_H__
#define __FPGA_PROCESS_EDGE_H__


#include "graph_fpga.h"

#include "fpga_raw_solver.h"

#define BURST_WRITE_SIZE        (64)
#define LOG2_BURST_WRITE_SIZE   (6)
//#define BURST_WRITE_BUFFER_SIZE (BURST_WRITE_SIZE *SIZE_BY_INT)
#define L 3


#define TOTAL_STEAM_READ_NUM    ((BURST_WRITE_SIZE * SIZE_BY_INT / 2)>> LOG2_PE_NUM)

void processEdgeWrite(
    uint_raw                sink_offset,
    uint_raw                sink_end,
    hls::stream<uint_uram>  (&writeArray)[PE_NUM],
    uint16                  *tmpVertexProp
)
{
    uint_raw dstEnd   = sink_end;


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

    for (int i = 0; i < ((MAX_VERTICES_IN_ONE_PARTITION) >> ( LOG2_BURST_WRITE_SIZE + LOG2_SIZE_BY_INT)); i++)
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

            tmpVertexProp[(i << LOG2_BURST_WRITE_SIZE) + burst_index ]
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
    uint_uram               tmpVPropBuffer[(MAX_VERTICES_IN_ONE_PARTITION / 2) >> LOG2_PE_NUM],
    //uint_raw                bitmap[BITMAP_SLICE_SIZE][BITMAP_SUB_SIZE],
    hls::stream<int2_token> &buildArray,
    hls::stream<uint_uram>  &writeArray
)
{
#pragma HLS function_instantiate variable=index
#pragma HLS dependence variable=tmpVPropBuffer inter false

    uint_raw dstEnd   = sink_end;

    uint_uram lastValue [L + 1];
    uint_raw lastIndex [L + 1];
#pragma HLS ARRAY_PARTITION variable=lastValue dim=0 complete
#pragma HLS ARRAY_PARTITION variable=lastIndex dim=0 complete

    for (int i = 0; i < (L + 1); i ++)
    {
        lastValue[i] = 0;
        lastIndex[i] = 0;
    }

    while (true) {
#pragma HLS PIPELINE II=2
#pragma HLS latency min=2 max=2

        int2 tmp_data;
        int2_token in_token;
        read_from_stream(buildArray, in_token);
        tmp_data = in_token.data;

        if ((in_token.flag == FLAG_SET))
        {
            break;
        }
        else
        {
            uint_raw idx;

            uint_raw dstVidx  = tmp_data.x;
            idx = (dstVidx >> LOG2_PE_NUM ) & ((MAX_VERTICES_IN_ONE_PARTITION >> LOG2_PE_NUM) - 1);
            uint_raw addr = (idx >> 1);

            if (dstVidx.range(31,31))
            {
                // Nothing here
            }
            else
            {
#pragma HLS latency min=1 max=2
                uint_uram updated_value = tmpVPropBuffer[addr];
                
                for (int i = 0; i < L + 1; i ++)
    #pragma HLS UNROLL
                {
                    if (lastIndex[i] == addr) updated_value = lastValue[i];
                }
                
                for (int i = 0; i < L; i ++)
    #pragma HLS UNROLL
                {
                    lastValue[i] = lastValue[i + 1];
                    lastIndex[i] = lastIndex[i + 1];
                }

                uint_uram temp_updated_value = updated_value;

                uint_raw msb = updated_value.range(63, 32);
                uint_raw lsb = updated_value.range(31, 0);

                uint_raw msb_out = PROP_COMPUTE_STAGE3(msb, tmp_data.y);
                uint_raw lsb_out = PROP_COMPUTE_STAGE3(lsb, tmp_data.y);

                uint_uram accumulate_msb;
                uint_uram accumulate_lsb;

                accumulate_msb.range(63, 32) = msb_out;
                accumulate_msb.range(31,  0) = temp_updated_value.range(31, 0);

                accumulate_lsb.range(63, 32) = temp_updated_value.range(63, 32);
                accumulate_lsb.range(31,  0) = lsb_out;

                if (idx & 0x01)
                {
                    tmpVPropBuffer[addr] = accumulate_msb;
                    lastValue[L] = accumulate_msb;
                }
                else
                {
                    tmpVPropBuffer[addr] = accumulate_lsb;
                    lastValue[L] = accumulate_lsb;
                }
                lastIndex[L] = addr;
            }
        }
    }
    
    for (int i = 0; i < ((MAX_VERTICES_IN_ONE_PARTITION ) >> (LOG2_PE_NUM + 1)); i++)
    {
#pragma HLS PIPELINE II=1
        uint_uram tmp = tmpVPropBuffer[i];
        write_to_stream(writeArray, tmp);
        if (i > 0)
            tmpVPropBuffer[i - 1] = 0;
    }
    tmpVPropBuffer[((MAX_VERTICES_IN_ONE_PARTITION ) >> (LOG2_PE_NUM + 1)) - 1] = 0;

}


#endif /* __FPGA_PROCESS_EDGE_H__ */
