
### API Table (under construction)
#### Stream operations
To provide more portability, ThunderGP warps some operations on steam channel

| Module    | Description  |
|-----------|--------------|
| read_from_stream    |  blocking read from stream   |
| read_from_stream_nb | nonblocking read from stream |
| write_to_stream     |  blocking write to stream    |
| clear_stream        |  clear the stream for next iteration |
| empty_stream        |  clear the stream in some unexpected situations |


#### L1 
| Module    | Description  |
|-----------|--------------|
| burstRead | Read data in bust mode from global memory and generate stream |
|burstReadLite|Similar to burstRead, but do not send the END_FLAG token|
| writeBack | write the stream data into global memory |
|writeBackLite| Similar to writeBack, but do not check the END_FLAG token|
| srcPropertyProcess | Manage the source vertex cache and output stream of edge tuples|
| shuffleDecoder   | Shuffle decoder |
| tupleFilter  | Filter valid data for PEs  |
| rawSolver  | Read-after-write(RAW) hazard solver |
| dstPropertyProcess | As gather PE, manage the destine destination vertices    |
|  cuMerge         | Merge all the intermediate result for ***apply*** stage               |
| cuDuplicate | Dispatch the updated results from ***apply*** to each group of gather-scatter kernels |

Example:

```c

/*simplified gather_scatter kernel*/
{
    burstRead(0, edge_end, edges, edgeBurstStream);
    burstRead(0, edge_end, edgeScoreMap, mapStream);
    burstRead(0, edge_end, edgeProp, edgePropStream);

    srcPropertyProcess(vertexScore, edgeBurstSliceStream, mapSliceStream, edgeTuplesBuffer);
    propProcess(edgePropSliceStream, edgeTuplesBuffer, edgeTuplesCoupled);
    for (int i = 0; i < PE_NUM ; i++)
    {
#pragma HLS UNROLL
        shuffleDecoder (
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
        tupleFilter( filter_tmp[i], filter_num[i], toFilterArraySlice2[i], buildArray[i]);
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
}
```
#### L2
| Hooks    | Description  |
|-----------|--------------|
| preprocessProperty | Per-process the source vertex property. |
| updateCalculation | Calculate the update value by using the edge property and source vertex property.  |
| updateMergeInRAWSolver | Destination property update in RAW solver. | 
| updateDestination | Destination property update. | 
| applyMerge | Destination property merge from all of the scatter-gather CUs. | 


Example:

```c

/* source vertex property & edge property */
prop_t updateCalculation(prop_t srcProp,prop_t edgeProp)    
{
    return((srcProp) + (edgeProp))
}
/* destination property update in RAW solver */
prop_t updateMergeInRAWSolver(prop_t ori,prop_t update)    
{
    return ((((ori)& (~VERTEX_ACTIVE_BIT_MASK)) > ((update) & (~VERTEX_ACTIVE_BIT_MASK)))?(update):(ori))
}
/* destination property update dst buffer update */
prop_t updateDestination(prop_t ori,prop_t update)   
{
    return (((((ori)& (~VERTEX_ACTIVE_BIT_MASK)) > ((update) & (~VERTEX_ACTIVE_BIT_MASK))) || (ori == 0x0))?(update):(ori))
}

/* destination property merge */
prop_t applyMerge(prop_t ori,prop_t update)   
{

    return ((((((ori)& (~VERTEX_ACTIVE_BIT_MASK)) > ((update) & (~VERTEX_ACTIVE_BIT_MASK))) && (update != 0)) || (ori == 0x0))?(update):(ori))
}
```

#### L3
