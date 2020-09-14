
# API Table

## L1 
### Stream Operations
To provide more portability, ThunderGP warps some operations on steam channel

| Function  | Description  |
|-----------|--------------|
| read_from_stream    |  blocking read from stream   |
| read_from_stream_nb | nonblocking read from stream |
| write_to_stream     |  blocking write to stream    |
| clear_stream        |  clear the stream for next iteration |
| empty_stream        |  clear the stream in some unexpected situations |

### Processing Modules
| Module    | Description  |
|-----------|--------------|
| burstRead | read data in bust mode from global memory and generate stream |
|burstReadLite|similar to burstRead, but do not send the END_FLAG token|
| writeBack | write the stream data into global memory |
|writeBackLite| similar to writeBack, but do not check the END_FLAG token|
| srcPropertyProcess | manage the source vertex cache and output stream of edge tuples|
| shuffleDecoder   | shuffle decoder |
| tupleFilter  | filter valid data for PEs  |
| rawSolver  | read-after-write(RAW) hazard solver |
| dstPropertyProcess | as gather PE, manage the destine destination vertices    |
|  cuMerge         | merge all the intermediate result for ***apply*** stage               |
| cuDuplicate | dispatch the updated results from ***apply*** to each group of gather-scatter kernels |

Example:

```c

/*simplified gather_scatter kernel*/
{
    burstRead(0, edge_end, edgesTailArray, edgeBurstStream);
    burstRead(0, edge_end, edgesHeadArray, mapStream);
    burstRead(0, edge_end, edgeProp, edgePropStream);

    srcPropertyProcess(vertexPushinProp, edgeBurstSliceStream, mapSliceStream, edgeTuplesBuffer);
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
## L2
| Hooks    | Description  |
|-----------|--------------|
| preprocessProperty | per-process the source vertex property. |
| scatterFunc | calculate the update value by using the edge property and source vertex property.  |
| updateMergeInRAWSolver | destination property update in RAW solver. | 
| gatherFunc | destination property update. | 
| applyMerge | destination property merge from all of the scatter-gather CUs. | 
| applyFunc | calculate the new property value |



Example:

```c

/* source vertex property process */
inline prop_t preprocessProperty(prop_t srcProp)
{
    return ((srcProp) + 1);
}

/* source vertex property & edge property */
inline prop_t scatterFunc(prop_t srcProp, prop_t edgeProp)
{
    return (srcProp);
}

/* destination property update in RAW solver */
inline prop_t updateMergeInRAWSolver(prop_t ori, prop_t update)
{
    return ((((ori) & (~VERTEX_ACTIVE_BIT_MASK)) > ((update) & (~VERTEX_ACTIVE_BIT_MASK))) ? (update) : (ori));
}

/* destination property update dst buffer update */
inline prop_t gatherFunc(prop_t ori, prop_t update)
{
    return (((((ori) & (~VERTEX_ACTIVE_BIT_MASK)) > ((update) & (~VERTEX_ACTIVE_BIT_MASK))) || (ori == 0x0)) ? (update) : (ori));
}

/* destination property merge */
inline prop_t applyMerge(prop_t ori, prop_t update)
{
    return ((((((ori) & (~VERTEX_ACTIVE_BIT_MASK)) > ((update) & (~VERTEX_ACTIVE_BIT_MASK))) && (update != 0)) || (ori == 0x0)) ? (update) : (ori));
}

inline prop_t applyFunc( prop_t tProp,
                                prop_t source,
                                prop_t outDeg,
                                unsigned int &extra,
                                unsigned int arg
                              )
{
    prop_t update = 0;

    prop_t uProp  = source;
    prop_t wProp;
    if ((uProp & 0x80000000) == (tProp & 0x80000000))
    {
        wProp = uProp & 0x7fffffff;  // last active vertex
    }
    else if ((tProp & 0x80000000) == 0x80000000)
    {
        extra ++;
        wProp = tProp; // current active vertex
    }
    else
    {
        wProp = MAX_PROP; // not travsered
    }
    update = wProp;

    return update;
}
```

## L3

### Partition Descriptor

| Function  | Description  |
|-----------|--------------|
| getSubPartition    | return the instance of sub-partition by the given ID   |
| getPartition |  return the instance of partition by the given ID   |

### Kernel Descriptor

| Function  | Description  |
|-----------|--------------|
| kernelInit    | initialize the OpenCL kernels  |
| getGatherScatter |  return the instance of gather-scatter kernel  |
|getApply|  return the instance of apply kernel |
|setGsKernel| setup the gather-scatter kernel for processing one partition data |
|setApplyKernel| setup the apply kernel for processing one partition data|


### Scheduler

| Function  | Description  |
|-----------|--------------|
|registerScheduler  | register the customized scheduler into ThunderGP |


### Accelerator


| Function  | Description  |
|-----------|--------------|
|acceleratorInit  | initialize the the bitstream and hardware|
|acceleratorDataLoad  | load the graph data |
|acceleratorDataPreprocess  | graph partitioning |
|acceleratorSuperStep  | process all of the partitions once |
|acceleratorProfile  | profile the execution time and verify the result |
|acceleratorDeinit  | release all the dynamic resources |

