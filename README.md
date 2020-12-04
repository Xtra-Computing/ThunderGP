![logo](docs/images/ThunderGP.png)

[![GitHub license](https://img.shields.io/badge/license-apache2-yellowgreen)](./LICENSE)
[![GitHub issues](https://img.shields.io/github/issues/Xtra-Computing/On-the-fly-data-shuffling-for-OpenCL-based-FPGAs.svg)](https://github.com/Xtra-Computing/On-the-fly-data-shuffling-for-OpenCL-based-FPGAs/issues)

# ThunderGP: HLS-based Graph Processing Framework on FPGAs

## What's new?

ThunderGP enables data scientists to enjoy the ***performance*** of FPGA-based graph processing without compromising ***programmability***. ***To our best knowledge and experiments, this is the fastest graph processing framework on HLS-based FPGAs.***  

ThunderGP is accepted to appear in [FPGA 2021](https://isfpga.org/)


## Get Started with ThunderGP

## Prerequisites
* The gcc-4.8 or above
* Tools:
    * Vitis 2020.1 Design Suite
    * SDAccel 2018.3 Design Suite
    * SDAccel 2019.2 Design Suite
* Evaluated platforms from Xilinx:
    * Alveo U50 Data Center Accelerator Card (Vitis 2020.1)
    * Alveo U200 Data Center Accelerator Card (SDAccel 2019.2)
    * Alveo U250 Data Center Accelerator Card (SDAccel 2019.2)
    * Virtex UltraScale+ FPGA VCU1525 Acceleration Development Kit (SDAccel 2018.3)

    
## Work with the build-in graph processing applications
ThunderGP currently has seven build-in graph algorithms: PageRank (PR), Sparse Matrix-Vector Multiplication (SpMV), Breadth-First Search (BFS), Single Source Shortest Path (SSSP), Closeness Centrality (CC), ArticleRank (AR), and Weakly Connected Component (WCC). 

The wanted application can be implemented by passing argument ```app=[the wanted algorithm]``` to ``` make ``` command.   
The below table is for quick reference of this argument.

| Argument    | Accelerated algorithm  |
|--------------|--------------|
| ```app=pr``` | PageRank (PR)|
| ```app=spmv``` | Sparse Matrix-vector Multiplication (SpMV) |
| ```app=bfs``` | Breadth First Search (BFS)|
| ```app=sssp``` | Single Source Shortest Path (SSSP)|
| ```app=cc``` | Closeness Centrality (CC)|
| ```app=ar``` | ArticleRank  (AR)|
| ```app=wcc``` | Weakly Connected Component  (WCC)|

Here is an example of implementing an accelerator for PR. 
```sh
$ cd ./
$ make cleanall
$ make app=pr all # make the host execution program and FPGA execution program for pagerank application. It takes time.
$ ./host [bitfile] [graph name] #e.g., ./host_graph_fpga _x/link/int/graph_fpga.hw.xilinx_vcu1525_xdma_201830_1.xclbin wiki-talk
```
#### More details: [Compiling ThunderGP ](docs/compile_arch.md)

## Results (performance)

Throughput (MTEPS) of different graph processing algorithms over datasets on VCU1525 platform.

|App.|PR|SPMV|BFS|SSSP|CC|AR|WCC |
|----|--|----|---|----|--|--|----|
|R21| 5,015|4,190|5,417|3,901|4,623|4,848|4,584 |
|R24| 4,599|3,781|3,437|3,430|4,339|4,486|4,328 |
|G24| 5,039|4,037|5,216|3,725|4,752|4,927|4,704 |
|G25| 4,464|3,615|4,660|3,343|4,344|4,389|4,356 |
|WT|  2,884|2,874|2,717|2,427|2,776|2,833|2,776 |
|MG|  4,454|3,883|4,939|3,699|4,077|4,285|4,088 |
|PK|  4,001|3,729|4,251|3,169|3,833|3,909|3,716 |
|WP|  3,030|2,994|3,112|2,491|2,993|2,931|2,894 |
|LJ|  3,186|3,003|3,408|2,623|3,113|3,081|3,099 |
|TW|  2,938|2,801|2,120|2,425|2,962|2,853|2,894 |


Throughput (MTEPS) of different graph processing algorithms over datasets on U250 platform. 


|App.|PR|SPMV|BFS|SSSP|CC|AR|WCC |
|----|--|----|---|----|--|--|----|
|R21  |4,669|5,056|6,028|4,879|4,783|4,667|4,901 |
|R24  |4,732|4,946|5,897|4,285|4,939|4,732|4,988 |
|G24  |5,040|5,305|5,772|4,428|3,705|5,040|5,303 |
|G25  |4,978|4,072|4,974|3,864|3,661|4,984|5,254 |
|WT   |2,251|2,938|2,630|2,583|2,369|2,253|2,405 |
|MG   |3,756|4,195|4,949|4,378|3,914|3,737|3,891 |
|PK   |3,630|4,372|4,629|3,927|3,865|3,662|3,841 |
|WP   |3,255|3,652|4,058|3,417|3,341|3,259|3,432 |
|LJ   |3,342|3,693|4,329|3,614|3,557|3,328|3,708 |
|TW   |3,538|3,959|3,671|3,585|3,759|3,533|3,806 |



* [More Results](docs/results.md)

## APIs (programmability) 
![auto](docs/images/automation.png)


Benefiting from the high level abstraction of HLS, our APIs natively support C/C++ languages.  
ThunderGraph covers three levels of API for implementation or further exploration. 
APIs in L1 and L2 are for building the accelerators, and APIs of L3 are for host program. Details are as below:

* L1 is used to construct the basic modules to build the compute kernels and the dataflow. 

* L2 provides hooks for mapping graph processing algorithms. 
    * [Mapping new graph analytic algorithms](docs/algorithm_mapping.md)  

* L3 provides the high-level APIs on host side to deploy or control graph processing accelerator. Since recent FPGAs usually consist of multiple (SLRs), L3 also wraps the partition scheduling and memory management interface for multiple SLRs. 

    * [Memory Management](docs/memory.md) 

    * [Scheduling across Multi SLRs](docs/scheduling.md) 

    * [Verification](docs/verification.md)

* More details: [ThunderGP APIs ](docs/api_details.md)


## Framework Overview

### The Adopted Computation Model
The Gather-Apply-Scatter (GAS) model is widely used for FPGA-based graph processing frameworks as computation model due to its extensibility to various graph processing algorithms. ThunderGP adopts a simplified version of GAS model by following work [*On-the-fly-data-shuffling-for-OpenCL-based-FPGAs*](https://www.comp.nus.edu.sg/~hebs/pub/fpl19-graph.pdf).
This model updates the vertex property by propagating from source vertex to destination vertex. The input for the model is an unordered set of directed edges of the graph. Undirected edges in a graph can be represented by a pair of directed edges. 

<img src="docs/images/GASmodel.png" alt="drawing" width="500"/>

The process per iteration mainly contains three stages: **Scatter**, **Gather**, and **Apply**. 

* In the  **Scatter** stage (shown in line 2 to 6), for each input edge with format ```<src, dst, weight>```, an update tuple is generated for the destination vertex of the edge. The update tuple is of the format ```<dst, value>```, where the *dst* is the destination vertex of the edge and *value* is generated by processing the vertex properties and edge weights. 
* In the **Gather** stage (shown in line 7 to 9), all the update tuples generated in the  **Scatter** stage are accumulated to update destination vertices. 
* The final **Apply** stage (shown in line 10 to 12) executes an apply function on all the vertices of the graph. 


### The Execution Flow of ThunderGP

![overview](docs/images/overview.png)

As shown in the above diagram, The edges in one partition are streamed into **Scatter** stage, For each edges, the property of source vertices will be fetched from the global memory by the per-fetching and the cache module, at the same time, the property of corresponding edge, or the weight of edge is loaded from global memory in stream, then these two value go through an *__algorithm-specific processing__* which return an update of the property of the destination vertex, finally, at the end of scatter stage, this update value and the destination of this edge is combined to create a update tuple. The update tuples are streamed into the shuffle stage which dispatches the tuples to corresponding gather processing engines(PEs). The **Gather** PEs *__accumulates__* the update value in local on-chip memory which is caching the property of destination vertices. After all the edges in this partition are processed, the cached data in gather PEs will be aggregated to the global memory. and the **Apply** stage which calls *__algorithm-specific function__* updates all the vertices for the next iteration.

### Future Work

* Application wrapper for high level platform (Spark, etc.)
* Hardware-accelerated query engine.
* Cycle-precision software simulation for the verification of dynamic modules(Cache, etc.) and channel depth tuning.
* Optimization for large scale graph. (distributed processing or HBM-based memory hierarchy)
