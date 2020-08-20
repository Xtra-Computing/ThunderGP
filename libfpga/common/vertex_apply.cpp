#include <hls_stream.h>
#include <string.h>
#include "graph_fpga.h"


#include "fpga_global_mem.h"


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
        for(int j = 0; j < APPLY_REF_ARRAY_SIZE; j++)
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

            prop_t  wProp    = applyCalculation( tProp, uProp, out_deg, tmpInfoArray[i],  argReg);
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
        outReg[0] = infoAggregate;
    }
}


extern "C" {
    void  vertexApply(
        uint16        *vertexProp,
        uint16        *tmpVertexProp1,
        uint16        *tmpVertexProp2,
        uint16        *tmpVertexProp3,
        uint16        *tmpVertexProp4,
        uint16        *newVertexProp1,
        uint16        *newVertexProp2,
        uint16        *newVertexProp3,
        uint16        *newVertexProp4,
#if HAVE_APPLY_OUTDEG
        uint16        *outDegree,
#endif
        int           *outReg,
        unsigned int  vertexNum,
        unsigned int  addrOffset,
        unsigned int  argReg
    )
    {



        /* cu1 */
#pragma HLS INTERFACE m_axi port=tmpVertexProp1 offset=slave bundle=gmem1 max_read_burst_length=64 num_write_outstanding=4
#pragma HLS INTERFACE s_axilite port=tmpVertexProp1 bundle=control

#pragma HLS INTERFACE m_axi port=newVertexProp1 offset=slave bundle=gmem1 max_read_burst_length=64 num_write_outstanding=4
#pragma HLS INTERFACE s_axilite port=newVertexProp1 bundle=control


        /* cu2 */
#pragma HLS INTERFACE m_axi port=tmpVertexProp2 offset=slave bundle=gmem2 max_read_burst_length=64 num_write_outstanding=4
#pragma HLS INTERFACE s_axilite port=tmpVertexProp2 bundle=control

#pragma HLS INTERFACE m_axi port=newVertexProp2 offset=slave bundle=gmem2 max_read_burst_length=64 num_write_outstanding=4
#pragma HLS INTERFACE s_axilite port=newVertexProp2 bundle=control


        /* cu3 */
#pragma HLS INTERFACE m_axi port=tmpVertexProp3 offset=slave bundle=gmem3 max_read_burst_length=64 num_write_outstanding=4
#pragma HLS INTERFACE s_axilite port=tmpVertexProp3 bundle=control

#pragma HLS INTERFACE m_axi port=newVertexProp3 offset=slave bundle=gmem3 max_read_burst_length=64 num_write_outstanding=4
#pragma HLS INTERFACE s_axilite port=newVertexProp3 bundle=control


        /* cu4 */
#pragma HLS INTERFACE m_axi port=tmpVertexProp4 offset=slave bundle=gmem4 max_read_burst_length=64 num_write_outstanding=4
#pragma HLS INTERFACE s_axilite port=tmpVertexProp4 bundle=control

#pragma HLS INTERFACE m_axi port=newVertexProp4 offset=slave bundle=gmem4 max_read_burst_length=64 num_write_outstanding=4
#pragma HLS INTERFACE s_axilite port=newVertexProp4 bundle=control



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

        hls::stream<burst_raw>      tmpVertexPropArray[4];
#pragma HLS stream variable=tmpVertexPropArray depth=2


        hls::stream<burst_raw>      tmpVertexPropStream;
#pragma HLS stream variable=tmpVertexPropStream depth=128



        hls::stream<burst_raw>      newVertexPropStream;
#pragma HLS stream variable=newVertexPropStream depth=32

        hls::stream<burst_raw>      newVertexPropArray[4];
#pragma HLS stream variable=newVertexPropArray depth=2

        int loopNum = (vertexNum >> 4) ;



        burstReadLite(addrOffset, vertexNum, vertexProp, vertexPropStream);


        burstReadLite(0, vertexNum, tmpVertexProp1, tmpVertexPropArray[0]);
        burstReadLite(0, vertexNum, tmpVertexProp2, tmpVertexPropArray[1]);
        burstReadLite(0, vertexNum, tmpVertexProp3, tmpVertexPropArray[2]);
        burstReadLite(0, vertexNum, tmpVertexProp4, tmpVertexPropArray[3]);

        cuMerge(loopNum, tmpVertexPropArray, tmpVertexPropStream);

        applyFunction(
            loopNum,
#if HAVE_APPLY_OUTDEG
            outDegreeStream,
#endif
            vertexPropStream,
            tmpVertexPropStream,
            argReg,
            newVertexPropStream,
            outReg
        );

        cuDuplicate(loopNum , newVertexPropStream,
                    newVertexPropArray);

        writeBackLite(vertexNum, newVertexProp1 + (addrOffset >> 4), newVertexPropArray[0]);
        writeBackLite(vertexNum, newVertexProp2 + (addrOffset >> 4), newVertexPropArray[1]);
        writeBackLite(vertexNum, newVertexProp3 + (addrOffset >> 4), newVertexPropArray[2]);
        writeBackLite(vertexNum, newVertexProp4 + (addrOffset >> 4), newVertexPropArray[3]);

    }

}
