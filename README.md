
## Prerequisites
* The gcc-4.8 or above
* The SDAccel 2018.3 Design Suit
* The Xilinx Virtex UltraScale+ FPGA VCU1525 Acceleration Development Kit

## Run the code

Do not forget to set the PATH of the dataset. 

```sh
$ cd ./
$ make clean all
$ make all -j # make the host execution program and FPGA execution program. It takes time.
$ ./host [bitfile] [graph name] #e.g., ./host_graph_fpga _x/link/int/graph_fpga.hw.xilinx_vcu1525_xdma_201830_1.xclbin wiki-talk
```
## API Definition
ThunderGraph covers three levels of API for implementation or further exploration. 
### Overview
both L1 and L2 is HLS code APIs for building the FPGA accelerators, and L3 is APIs for host program.
#### L1
L1 provides the fundamental functions and basic modules used in our framework to build the compute kernels and the data flow. Users can use these API to construct their own data flow as well.

#### L2
L2 provides hooks for mapping graph processing algorithm. As we have shown our proposed data flow is efficient (refer to our paper), in this level, the data flow is fixed, and we only focus on how to map graph processing algorithm smoothly.

The figure blow show the data flow of our framework and the L2 hooks,There are two main compute unit: ***gather-scatter*** kernel and ***apply*** kernel for mapping a new algorithm, users need to handle the following hooks, calcuationUpdateValue, updateProperty etc.

#### L3
L3 provides the high level software APIs to deploy and control graph processing engine. There is a tend to using multiple-SLRs FPGA or multiple FPGAs cluster in the future heterogeneous system, therefore, L3 also wraps the partition scheduling and memory management interface for further exploration. 

### API Table (under construction)
#### L1 
| Module    | Description  |
|-----------|--------------|
| burstRead | Read data in bust mode from global memory and generate stream |
| writeBack | write the stream data into global memory |
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
| updateCalculation | Calculate the update value by using the edge property and source vertex property  |
| updateMergeInRAWSolver | destination property update in RAW solver | 
| updateDestination | destination property update | 
| applyMerge | destination property merge from other group | 


Example:

```c

/* source vertex property & edge property */
unsigned int updateCalculation(srcProp, edgeProp)    
{
    return((srcProp) + (edgeProp))
}
/* destination property update in RAW solver */
unsigned int updateMergeInRAWSolver(ori, update)    
{
    return ((((ori)& (~VERTEX_ACTIVE_BIT_MASK)) > ((update) & (~VERTEX_ACTIVE_BIT_MASK)))?(update):(ori))
}
/* destination property update dst buffer update */
unsigned int updateDestination(ori,update)   
{
    return (((((ori)& (~VERTEX_ACTIVE_BIT_MASK)) > ((update) & (~VERTEX_ACTIVE_BIT_MASK))) || (ori == 0x0))?(update):(ori))
}

/* destination property merge */
unsigned int applyMerge(ori,update)   
{

    return ((((((ori)& (~VERTEX_ACTIVE_BIT_MASK)) > ((update) & (~VERTEX_ACTIVE_BIT_MASK))) && (update != 0)) || (ori == 0x0))?(update):(ori))
}
```

#### L3
