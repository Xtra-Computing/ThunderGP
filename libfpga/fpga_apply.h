#include <hls_stream.h>
#include <string.h>
#include "graph_fpga.h"


#include "fpga_global_mem.h"


template <typename T>
void  cuDuplicate ( int               loopNum,
                    hls::stream<T>    &input,
                    hls::stream<T>    (&output)[SUB_PARTITION_NUM])
{
#pragma HLS function_instantiate variable=input
    for (int i = 0; i < loopNum ; i++)
    {
#pragma HLS PIPELINE II=1
        T  unit;
        read_from_stream(input, unit);
        for (int j = 0; j < SUB_PARTITION_NUM; j ++)
        {
#pragma HLS UNROLL
            write_to_stream(output[j], unit);
        }
    }
}



template <typename T>
void  cuMerge ( int               loopNum,
                hls::stream<T>    &input_a,
                hls::stream<T>    &input_b,
                hls::stream<T>    &output)
{
#pragma HLS function_instantiate variable=input_a
    for (int i = 0; i < loopNum ; i++)
    {
#pragma HLS PIPELINE II=1
        T  unit[2];
#pragma HLS ARRAY_PARTITION variable=unit dim=0 complete


        read_from_stream(input_a, unit[0]);
        read_from_stream(input_b, unit[1]);

        T res;
        for (int inner = 0; inner < 16 ; inner ++)
        {
#pragma HLS UNROLL
            uint_raw tmp = PROP_COMPUTE_STAGE4(
                                unit[0].range(31 + inner * 32, 0 + inner * 32),
                                unit[1].range(31 + inner * 32, 0 + inner * 32)
                            );
            res.range(31 + inner * 32, 0 + inner * 32) = tmp;
        }

        write_to_stream(output, res);
    }
}

#if (CUSTOMIZE_APPLY==0)

void applyFunction(
    int                             loopNum,
#if HAVE_APPLY_OUTDEG
    hls::stream<burst_raw>          &outDegreeStream,
#endif
    hls::stream<burst_raw>          &vertexPropStream,
    hls::stream<burst_raw>          &tmpVertexPropStream,
    unsigned int                    argReg,
    hls::stream<burst_raw>          &newVertexPropStream,
    int                             *outReg
)
{
    unsigned int infoArray[BURST_ALL_BITS / INT_WIDTH][APPLY_REF_ARRAY_SIZE];
#pragma HLS ARRAY_PARTITION variable=infoArray dim=0 complete
    for (int i = 0; i < BURST_ALL_BITS / INT_WIDTH; i++)
    {
        for (int j = 0; j < APPLY_REF_ARRAY_SIZE; j++)
        {
            infoArray[i][j] = 0;
        }
    }
    for (int loopCount = 0; loopCount < loopNum; loopCount ++)
    {

#pragma HLS PIPELINE II=1
        burst_raw vertexProp;
        burst_raw tmpVertexProp;

        read_from_stream(vertexPropStream, vertexProp);
        read_from_stream(tmpVertexPropStream, tmpVertexProp);

#if HAVE_APPLY_OUTDEG
        burst_raw outDeg;
        read_from_stream(outDegreeStream, outDeg);
#endif

        burst_raw newVertexProp;

        for (int i = 0; i < BURST_ALL_BITS / INT_WIDTH; i++)
        {
#pragma HLS UNROLL
            prop_t tProp     = tmpVertexProp.range((i + 1) * INT_WIDTH - 1, i * INT_WIDTH );
            prop_t uProp     = vertexProp.range(   (i + 1) * INT_WIDTH - 1, i * INT_WIDTH );
#if HAVE_APPLY_OUTDEG
            prop_t out_deg   = outDeg.range(       (i + 1) * INT_WIDTH - 1, i * INT_WIDTH );
#else
            prop_t out_deg   = 0;
#endif
            unsigned int tmpInfoArray[BURST_ALL_BITS / INT_WIDTH][APPLY_REF_ARRAY_SIZE];
#pragma HLS ARRAY_PARTITION variable=tmpInfoArray dim=0 complete
//#pragma HLS DEPENDENCE variable=tmpInfoArray inter false

            prop_t  wProp    = applyFunc( tProp, uProp, out_deg, tmpInfoArray[i],  argReg);
            for (int j = 0; j < APPLY_REF_ARRAY_SIZE; j++)
            {
                infoArray[i][j] += tmpInfoArray[i][j];
            }
            newVertexProp.range((i + 1) * INT_WIDTH - 1, i * INT_WIDTH ) = wProp;

        }
        write_to_stream(newVertexPropStream, newVertexProp);
    }

    for (int j = 0; j < APPLY_REF_ARRAY_SIZE; j++)
    {
        int infoAggregate = 0;

        for (int i = 0; i < BURST_ALL_BITS / INT_WIDTH; i ++)
        {
            DEBUG_PRINTF("infoArray %d %d \n", i, infoArray[i]);
            infoAggregate += infoArray[i][j];
        }
        outReg[j] = infoAggregate;
    }
}
#endif
