
{
#pragma HLS INTERFACE m_axi port=edgeScoreMap offset=slave bundle=gmem0 max_read_burst_length=64
#pragma HLS INTERFACE s_axilite port=edgeScoreMap bundle=control

#pragma HLS INTERFACE m_axi port=edges offset=slave bundle=gmem2 max_read_burst_length=64
#pragma HLS INTERFACE s_axilite port=edges bundle=control

#pragma HLS INTERFACE m_axi port=tmpVertexProp offset=slave bundle=gmem1 max_read_burst_length=64 num_write_outstanding=4
#pragma HLS INTERFACE s_axilite port=tmpVertexProp bundle=control

#pragma HLS INTERFACE m_axi port=vertexScore offset=slave bundle=gmem1 max_read_burst_length=64 num_write_outstanding=4
#pragma HLS INTERFACE s_axilite port=vertexScore bundle=control

#if HAVE_EDGE_PROP

#pragma HLS INTERFACE m_axi port=edgeProp offset=slave bundle=gmem3 max_read_burst_length=64
#pragma HLS INTERFACE s_axilite port=edgeProp bundle=control

#endif
#pragma HLS INTERFACE s_axilite port=edge_end       bundle=control
#pragma HLS INTERFACE s_axilite port=sink_offset    bundle=control
#pragma HLS INTERFACE s_axilite port=sink_end       bundle=control
#pragma HLS INTERFACE s_axilite port=return         bundle=control



    hls::stream<int2>           buildArray[PE_NUM];
#pragma HLS stream variable=buildArray  depth=2

    hls::stream<int2>           buildArrayRes[PE_NUM];
#pragma HLS stream variable=buildArrayRes  depth=2

    hls::stream<int2>           buildArraySlice[PE_NUM];
#pragma HLS stream variable=buildArraySlice  depth=2

    hls::stream<edge_tuples_t>   edgeTuplesArray[PE_NUM];
#pragma HLS stream variable=edgeTuplesArray  depth=2

    hls::stream<filter_type>    toFilterArray[PE_NUM];
#pragma HLS stream variable=toFilterArray  depth=32

    hls::stream<filter_type>    toFilterArraySlice[PE_NUM];
#pragma HLS stream variable=toFilterArraySlice  depth=4

    hls::stream<filter_type>    toFilterArraySlice2[PE_NUM];
#pragma HLS stream variable=toFilterArraySlice2  depth=4


    hls::stream<uint_uram>    writeArray[PE_NUM];
#pragma HLS stream variable=writeArray  depth=2

    hls::stream<uint_uram>    writeArrayLayer2[PE_NUM];
#pragma HLS stream variable=writeArrayLayer2  depth=2

    hls::stream<uint_uram>    writeArrayLayer1[PE_NUM];
#pragma HLS stream variable=writeArrayLayer1  depth=2


    hls::stream<burst_raw>      edgeBurstSliceStream;
#pragma HLS stream variable=edgeBurstSliceStream depth=2
    hls::stream<burst_raw>      edgeBurstStream;
#pragma HLS stream variable=edgeBurstStream depth=2
    hls::stream<burst_raw>      edgeBurstStreamTmp;
#pragma HLS stream variable=edgeBurstStreamTmp depth=512

    hls::stream<burst_raw>      mapSliceStream;
#pragma HLS stream variable=mapSliceStream depth=2
    hls::stream<burst_raw>      mapStream;
#pragma HLS stream variable=mapStream depth=2
    hls::stream<burst_raw>      mapStreamTmp;
#pragma HLS stream variable=mapStreamTmp depth=512

#if HAVE_EDGE_PROP
    hls::stream<burst_raw>      edgePropSliceStream;
#pragma HLS stream variable=edgePropSliceStream depth=2
    hls::stream<burst_raw>      edgePropStream;
#pragma HLS stream variable=edgePropStream depth=2
    hls::stream<burst_raw>      edgePropStreamTmp;
#pragma HLS stream variable=edgePropStreamTmp depth=512

#endif

    hls::stream<edge_tuples_t>   edgeTuplesCoupled;
#pragma HLS stream variable=edgeTuplesCoupled depth=2

    hls::stream<edge_tuples_t>   edgeTuplesBuffer;
#pragma HLS stream variable=edgeTuplesBuffer depth=2

    hls::stream<edge_tuples_t>   edgeTuplesLayer1[4];
#pragma HLS stream variable=edgeTuplesLayer1 depth=2


    edge_tuples_t tuples[PE_NUM];
#pragma HLS ARRAY_PARTITION variable=tuples dim=0 complete

    filter_type filter[PE_NUM];
#pragma HLS ARRAY_PARTITION variable=filter dim=0 complete

    uchar opcode[PE_NUM];
#pragma HLS ARRAY_PARTITION variable=opcodep dim=0 complete

    shuffled_type shuff_ifo[PE_NUM];
#pragma HLS ARRAY_PARTITION variable=shuff_ifo dim=0 complete


    uint_uram tmpVPropBuffer[PE_NUM][(VERTEX_MAX / 2) >> (LOG2_PE_NUM)];
#pragma HLS ARRAY_PARTITION variable=tmpVPropBuffer dim=1 complete
#pragma HLS RESOURCE variable=tmpVPropBuffer core=XPM_MEMORY uram



    filter_type                 filter_tmp[PE_NUM];
#pragma HLS ARRAY_PARTITION variable=filter_tmp dim=0 complete
    uint_raw                    filter_num[PE_NUM];
#pragma HLS ARRAY_PARTITION variable=filter_num dim=0 complete

#pragma HLS DATAFLOW
    //printf("%d %d \n",(int)edge_offset,(int)edge_end );
    burstRead(0, edge_end, edges, edgeBurstStream);
    sliceStream(edgeBurstStream, edgeBurstStreamTmp);
    sliceStream(edgeBurstStreamTmp, edgeBurstSliceStream);

    burstRead(0, edge_end, edgeScoreMap, mapStream);
    sliceStream(mapStream, mapStreamTmp);
    sliceStream(mapStreamTmp, mapSliceStream);

#if HAVE_EDGE_PROP
    
    burstRead(0, edge_end, edgeProp, edgePropStream);
    sliceStream(edgePropStream, edgePropStreamTmp);
    sliceStream(edgePropStreamTmp, edgePropSliceStream);

#endif

    srcPropertyProcess(vertexScore, edgeBurstSliceStream, mapSliceStream, edgeTuplesBuffer);

#if HAVE_EDGE_PROP
    propProcess(edgePropSliceStream, edgeTuplesBuffer, edgeTuplesCoupled);
#else
    propProcessSelf(edgeTuplesBuffer, edgeTuplesCoupled);
#endif
    /* timing */
    duplicateStream4WithClear(edgeTuplesCoupled, edgeTuplesLayer1[0], edgeTuplesLayer1[1], edgeTuplesLayer1[2], edgeTuplesLayer1[3]);

    for (int i = 0; i < 4 ; i++)
    {
#pragma HLS UNROLL
        duplicateStream4(edgeTuplesLayer1[i],
        edgeTuplesArray[i * 4 + 0],
        edgeTuplesArray[i * 4 + 1],
        edgeTuplesArray[i * 4 + 2],
        edgeTuplesArray[i * 4 + 3] );
    }

#if 1
    for (int i = 0; i < PE_NUM ; i++)
    {
#pragma HLS UNROLL
        shuffleEntry (
            i,
            edgeTuplesArray[i],
            toFilterArraySlice[i],
            filter[i],
            tuples[i],
            opcode[i],
            shuff_ifo[i]
        );
    }


    for (int i = 0; i < PE_NUM ; i++)
    {
#pragma HLS UNROLL
        filterSlice(toFilterArraySlice[i], toFilterArray[i]);
    }
    for (int i = 0; i < PE_NUM ; i++)
    {
#pragma HLS UNROLL
        filterSlice(toFilterArray[i], toFilterArraySlice2[i]);
    }
//    for (int i = 0; i < PE_NUM ; i++)
//    {
//#pragma HLS UNROLL
//        filterSlice(toFilterArraySlice3[i], toFilterArraySlice[i]);
//    }

    for (int i = 0; i < PE_NUM ; i++)
    {
#pragma HLS UNROLL
        tupleFilter( filter_tmp[i], filter_num[i], toFilterArraySlice2[i], buildArray[i]);
    }


    for (int i = 0; i < PE_NUM ; i++)
    {
#pragma HLS UNROLL
        processEdgesBuildSlice(buildArray[i], buildArraySlice[i]);
    }
    for (int i = 0; i < PE_NUM ; i++)
    {
#pragma HLS UNROLL
        rawSolver(buildArraySlice[i], buildArrayRes[i]);
    }
    for (int i = 0; i < PE_NUM ; i++)
    {
#pragma HLS UNROLL
        dstPropertyProcess(i, sink_offset, sink_end, tmpVPropBuffer[i], buildArrayRes[i], writeArrayLayer1[i]);
    }
    for (int i = 0; i < PE_NUM ; i++)
    {
#pragma HLS UNROLL
        processEdgesSlice(writeArrayLayer1[i], writeArray[i]);
    }

//    for (int i = 0; i < PE_NUM ; i++)
//    {
//#pragma HLS UNROLL
//        processEdgesSlice(writeArrayLayer2[i], writeArray[i]);
//    }

    processEdgeWrite(sink_offset, sink_end, writeArray, tmpVertexProp);

#endif

}