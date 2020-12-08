![logo](docs/images/ThunderGP.png)

[![GitHub license](https://img.shields.io/badge/license-apache2-yellowgreen)](./LICENSE)
[![GitHub issues](https://img.shields.io/github/issues/Xtra-Computing/On-the-fly-data-shuffling-for-OpenCL-based-FPGAs.svg)](https://github.com/Xtra-Computing/On-the-fly-data-shuffling-for-OpenCL-based-FPGAs/issues)

# ThunderGP: HLS-based Graph Processing Framework on FPGAs

## What's New?

ThunderGP enables data scientists to enjoy the ***performance*** of FPGA-based graph processing without compromising ***programmability***. ***To our best knowledge and experiments, this is the fastest graph processing framework on HLS-based FPGAs.***  

ThunderGP is accepted to appear in [FPGA 2021](https://isfpga.org/)


## Get Started with ThunderGP

### Prerequisites
* The gcc-4.8 or above
* Development environment:
    * Vitis 2020.1 Design Suite
    * SDAccel 2018.3 Design Suite
    * SDAccel 2019.2 Design Suite
* Evaluated platforms from Xilinx:
    * Alveo U50 Data Center Accelerator Card (Vitis 2020.1)
    * Alveo U200 Data Center Accelerator Card (SDAccel 2019.2)
    * Alveo U250 Data Center Accelerator Card (SDAccel 2019.2)
    * Virtex UltraScale+ FPGA VCU1525 Acceleration Development Kit (SDAccel 2018.3)
    
### Work with Build-in Graph Processing Applications
ThunderGP currently has seven build-in graph algorithms: PageRank (PR), Sparse Matrix-Vector Multiplication (SpMV), Breadth-First Search (BFS), Single Source Shortest Path (SSSP), Closeness Centrality (CC), ArticleRank (AR), and Weakly Connected Component (WCC). 
The desired application can be implemented by passing argument ```app=[the algorithm]``` to ``` make ``` command. The below table is for quick reference.

| Argument    | Accelerated algorithm  |
|--------------|--------------|
| ```app=pr``` | PageRank (PR)|
| ```app=spmv``` | Sparse Matrix-vector Multiplication (SpMV) |
| ```app=bfs``` | Breadth First Search (BFS)|
| ```app=sssp``` | Single Source Shortest Path (SSSP)|
| ```app=cc``` | Closeness Centrality (CC)|
| ```app=ar``` | ArticleRank  (AR)|
| ```app=wcc``` | Weakly Connected Component  (WCC)|

#### Here is the example of implementing the accelerator for PageRank on Alveo U50 platform with Vitis 2020.1. 
```sh
$ git clone https://github.com/Xtra-Computing/ThunderGP.git
$ git checkout develop_u50
$ cd ./
$ vim ThunderGP.mk 
$ # configure the DEVICE as DEVICES := xilinx_u50_gen3x16_xdma_201920_3; configure TARGETS := hw
$ make app=pr clean 
$ make app=pr all # make the host execution program and the FPGA bitstream. It takes time :)
# For execution on real hardware. The path of graph dataset needs to be provided by the user. 
$ ./host_graph_fpga_pr xclbin_pr/graph_fpga.hw.xilinx_u50_gen3x16_xdma_201920_3.xclbin wiki-talk
```
#### More details: [Compiling ThunderGP ](docs/compile_arch.md); [Performance of Seven Applications on Different Xilinx Platforms](docs/results.md)

### Build Your Own Graph processing Accelerators with ThunderGP
ThunderGP provides two sets of C++ based APIs: accelerator APIs (Acc-APIs) for customizing accelerators for graph algorithms and Host-APIs for accelerator deployment and execution.

* Acc-APIs provide hooks for mapping graph processing algorithms.  [Unfold the Detailed Instructions in Customize New Graph Accelerators with Acc-APIs](docs/algorithm_mapping.md)  

* Host-APIs deploy or control the graph processing accelerator. Since recent FPGAs usually consist of multiple (SLRs), It also wraps the partition scheduling and memory management interface for multiple SLRs. For detaiils, please check the following links. [Memory Management](docs/memory.md); [Scheduling across Multi SLRs](docs/scheduling.md)
; [Verification](docs/verification.md)

#### More details: [ThunderGP APIs ](docs/api_details.md)


## ThunderGP Design Details
![auto](docs/images/automation.png)  

The overview of ThunderGP is shown in the above figure. We briefly illustrate the main building blocks as follows.
* **Build-in accelerator template.** ThunderGP adopts the Gather-Apply-Scatter (GAS) model as the abstraction of various graph algorithms and realizes the model by a build-in highly-paralleled and memory-efficient accelerator template.
* **Automated accelerator generation.** The automated accelerator generation produces synthesizable accelerators with unleashing the full potentials of the underlying FPGA platform. In addition to the build-in accelerator template, it takes the user-defined functions (UDFs) of the scatter, the gather, and the apply stages (from the GAS model) of the graph algorithm and the FPGA platform model (e.g., U50)  from developers as inputs.
* **Graph partitioning and scheduling.** ThunderGP adopts a vertical partitioning method based on destination vertex without introducing heavy preprocessing operations such as edge-sorting to enable vertex buffering with on-chip RAMs.
* **High-level APIs.** ThunderGP provides two sets of C++ based APIs: accelerator APIs (Acc-APIs) for customizing accelerators for graph algorithms and Host-APIs for accelerator deployment and execution.


### The GAS Model
The Gather-Apply-Scatter (GAS) model is widely used for FPGA-based graph processing frameworks as computation model due to its extensibility to various graph processing algorithms. ThunderGP adopts a simplified version of GAS model by following work [*On-the-fly-data-shuffling-for-OpenCL-based-FPGAs*](https://www.comp.nus.edu.sg/~hebs/pub/fpl19-graph.pdf).
This model updates the vertex property by propagating from source vertex to destination vertex. The input for the model is an unordered set of directed edges of the graph. Undirected edges in a graph can be represented by a pair of directed edges. 

<img src="docs/images/GASmodel.png" alt="drawing" width="500"/>

The process per iteration mainly contains three stages: **Scatter**, **Gather**, and **Apply**. 

* In the  **Scatter** stage (shown in line 2 to 6), for each input edge with format ```<src, dst, weight>```, an update tuple is generated for the destination vertex of the edge. The update tuple is of the format ```<dst, value>```, where the *dst* is the destination vertex of the edge and *value* is generated by processing the vertex properties and edge weights. 
* In the **Gather** stage (shown in line 7 to 9), all the update tuples generated in the  **Scatter** stage are accumulated to update destination vertices. 
* The final **Apply** stage (shown in line 10 to 12) executes an apply function on all the vertices of the graph. 


### The Execution Flow of ThunderGP

![overview](docs/images/overview.png)  

As shown in the above diagram, the edges in one partition are streamed into **Scatter** stage, For each edges, the property of source vertices will be fetched from the global memory by the per-fetching and the cache module, at the same time, the property of corresponding edge, or the weight of edge is loaded from global memory in stream, then these two value go through an *__algorithm-specific processing__* which return an update of the property of the destination vertex, finally, at the end of scatter stage, this update value and the destination of this edge is combined to create a update tuple. The update tuples are streamed into the shuffle stage which dispatches the tuples to corresponding gather processing engines(PEs). The **Gather** PEs *__accumulates__* the update value in local on-chip memory which is caching the property of destination vertices. After all the edges in this partition are processed, the cached data in gather PEs will be aggregated to the global memory. and the **Apply** stage which calls *__algorithm-specific function__* updates all the vertices for the next iteration.

## Future Work
* Application wrapper for high level platform (Spark, etc.)
* Hardware-accelerated query engine.
* Cycle-precision software simulation for the verification of dynamic modules(Cache, etc.) and channel depth tuning.
* Optimization for large scale graph. (distributed processing)
