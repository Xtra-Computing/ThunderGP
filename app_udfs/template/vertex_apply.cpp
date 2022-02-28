#include <hls_stream.h>
#include <string.h>
#include "graph_fpga.h"


#include "fpga_global_mem.h"


void apply(
    int                             loopNum,
    hls::stream<burst_raw>          &outDegreeStream,
    hls::stream<burst_raw>          &vertexPropStream,
    hls::stream<burst_raw>          &tmpVertexPropStream,
    unsigned int                    baseScore ,
    hls::stream<burst_raw>          &newVertexPropStream,
    int                             *error
)
{
    int error_l[BURST_ALL_BITS / INT_WIDTH];
#pragma HLS ARRAY_PARTITION variable=error_l dim=0 complete
    for (int i = 0; i < BURST_ALL_BITS / INT_WIDTH; i++)
    {
        error_l[i] = 0;
    }
    for (int loopCount = 0; loopCount < loopNum; loopCount ++)
    {

#pragma HLS PIPELINE II=1
        burst_raw vertexProp;
        burst_raw tmpVertexProp;
        burst_raw outDeg;
        read_from_stream(vertexPropStream, vertexProp);
        read_from_stream(tmpVertexPropStream, tmpVertexProp);
        read_from_stream(outDegreeStream, outDeg);


        burst_raw newVertexProp;

        for (int i = 0; i < BURST_ALL_BITS / INT_WIDTH; i++)
        {
#pragma HLS UNROLL
            int tProp     = tmpVertexProp.range((i + 1) * INT_WIDTH - 1, i * INT_WIDTH );
            int old_score = vertexProp.range(   (i + 1) * INT_WIDTH - 1, i * INT_WIDTH );
            int new_score = baseScore  + ((kDampFixPoint * tProp) >> 7);
            int out_deg   = outDeg.range(       (i + 1) * INT_WIDTH - 1, i * INT_WIDTH );
            int update = 0;
            if (out_deg != 0)
            {
                update = new_score / out_deg;
            }
            else
            {
                update = 0;
            }

            newVertexProp.range((i + 1) * INT_WIDTH - 1, i * INT_WIDTH ) = update;
            //debug[i] = (new_score - old_score) > 0 ? (new_score - old_score) : (old_score - new_score);
            error_l[i] += (new_score - old_score) > 0 ? (new_score - old_score) : (old_score - new_score) ;
        }
        write_to_stream(newVertexPropStream, newVertexProp);

    }
    int total_error = 0;

    for (int i = 0; i < BURST_ALL_BITS / INT_WIDTH; i ++)
    {
        DEBUG_PRINTF("error_l %d %d \n", i, error_l[i]);
        total_error += error_l[i];
    }
    error[0] = total_error;

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
        uint16        *outDegree,
        int           *error,
        unsigned int  vertexNum,
        unsigned int  addrOffset,
        unsigned int  baseScore
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



#pragma HLS INTERFACE m_axi port=error offset=slave bundle=gmem5
#pragma HLS INTERFACE s_axilite port=error bundle=control


#pragma HLS INTERFACE m_axi port=vertexProp offset=slave bundle=gmem6 max_read_burst_length=64
#pragma HLS INTERFACE s_axilite port=vertexProp bundle=control


#pragma HLS INTERFACE m_axi port=outDegree offset=slave bundle=gmem7 max_read_burst_length=64 
#pragma HLS INTERFACE s_axilite port=outDegree bundle=control


#pragma HLS INTERFACE s_axilite port=vertexNum      bundle=control
#pragma HLS INTERFACE s_axilite port=baseScore      bundle=control
#pragma HLS INTERFACE s_axilite port=addrOffset     bundle=control
#pragma HLS INTERFACE s_axilite port=return         bundle=control

#pragma HLS DATAFLOW

        hls::stream<burst_raw>      vertexPropStream;
#pragma HLS stream variable=vertexPropStream depth=128

        hls::stream<burst_raw>      tmpVertexPropArray[4];
#pragma HLS stream variable=tmpVertexPropArray depth=2


        hls::stream<burst_raw>      tmpVertexPropStream;
#pragma HLS stream variable=tmpVertexPropStream depth=128

        hls::stream<burst_raw>      outDegreeStream;
#pragma HLS stream variable=outDegreeStream depth=256


        hls::stream<burst_raw>      newVertexPropStream;
#pragma HLS stream variable=newVertexPropStream depth=128

        hls::stream<burst_raw>      newVertexPropArray[4];
#pragma HLS stream variable=newVertexPropArray depth=2

        int loopNum = (vertexNum >> 4) ;



        burstReadLite(addrOffset, vertexNum, vertexProp, vertexPropStream);
        burstReadLite(addrOffset, vertexNum, outDegree, outDegreeStream);


        burstReadLite(0, vertexNum, tmpVertexProp1, tmpVertexPropArray[0]);
        burstReadLite(0, vertexNum, tmpVertexProp2, tmpVertexPropArray[1]);
        burstReadLite(0, vertexNum, tmpVertexProp3, tmpVertexPropArray[2]);
        burstReadLite(0, vertexNum, tmpVertexProp4, tmpVertexPropArray[3]);

        cuMerge(loopNum, tmpVertexPropArray, tmpVertexPropStream);

        apply(
            loopNum,
            outDegreeStream,
            vertexPropStream,
            tmpVertexPropStream,
            baseScore,
            newVertexPropStream,
            error
        );

        cuDuplicate(loopNum , newVertexPropStream,
                    newVertexPropArray);

        writeBackLite(vertexNum, newVertexProp1 + (addrOffset >> 4), newVertexPropArray[0]);
        writeBackLite(vertexNum, newVertexProp2 + (addrOffset >> 4), newVertexPropArray[1]);
        writeBackLite(vertexNum, newVertexProp3 + (addrOffset >> 4), newVertexPropArray[2]);
        writeBackLite(vertexNum, newVertexProp4 + (addrOffset >> 4), newVertexPropArray[3]);

    }

}
