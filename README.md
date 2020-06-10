![logo](docs/images/ThunderGP.png)

[![GitHub license](https://img.shields.io/badge/license-apache2-yellowgreen)](./LICENSE)
[![GitHub issues](https://img.shields.io/github/issues/Xtra-Computing/On-the-fly-data-shuffling-for-OpenCL-based-FPGAs.svg)](https://github.com/Xtra-Computing/On-the-fly-data-shuffling-for-OpenCL-based-FPGAs/issues)

# ThunderGP: An Efficient Graph Processing Framework for HLS-based FPGAs

## Introduction

ThunderGP enables data scientists to enjoy the ***performance*** of FPGA-based graph processing without compromising ***programmability***.

Two aspacts make the ThunderGP deliver superior performance.
On the one hand, ThunderGP embraces an improved execution flow to better exploit the pipeline parallelism of FPGA and alleviate the data access amount to the global memory. On the other hand, the memory accesses are highly optimized to fully utilize the memory bandwidth capacity of the hardware platforms. 

ThunderGP can run on both Xilinx and Intel platforms:

* [Check the implementation on Intel platform out.](https://github.com/Xtra-Computing/On-the-fly-data-shuffling-for-OpenCL-based-FPGAs/)

* On Xilinx multi-SLR based FPGAs, it is running at 250Mhz, and the performance can be up to ***5300 MTEPS (million traversed edges per second)***, or a ***2 times speedup*** over the state-of-the-art.


## Prerequisites
* The gcc-4.8 or above
* Tools:
    * SDAccel 2018.3 Design Suit
    * SDAccel 2019.2 Design Suit
* Evaluated platforms from Xilinx:
    * Xilinx Virtex UltraScale+ FPGA VCU1525 Acceleration Development Kit (SDAccel 2018.3)
    * Alveo U200 Data Center Accelerator Card (SDAccel 2019.2)
    * Alveo U250 Data Center Accelerator Card (SDAccel 2019.2)
    
## Run the code
Currently, ThunderGP supports four build-in graph analytic algorithms, namely PR, SpMV, BFS and SSSP.  
The wanted application can be implemented by passing argument ```app=[the wanted algorithm]``` to ``` make ``` command.   
The below table is for quick reference of this argument.

| Argument    | Accelerated algorithm  |
|--------------|--------------|
| ```app=pr``` | PageRank (PR)|
| ```app=spmv``` | Sparse Matrix-vector Multiplication (SpMV) |
| ```app=bfs``` | Breadth First Search (BFS)|
| ```app=sssp``` | Single Source Shortest Path (SSSP)|

Here is an example of implementing PR algorithm. 
```sh
$ cd ./
$ make cleanall
$ make app=pr all -j # make the host execution program and FPGA execution program for pagerank application. It takes time.
$ ./host [bitfile] [graph name] #e.g., ./host_graph_fpga _x/link/int/graph_fpga.hw.xilinx_vcu1525_xdma_201830_1.xclbin wiki-talk
```
#### More details: [Compiling ThunderGP ](docs/compile_arch.md)

## Results (performance)

Throughput (MTEPS) of different graph processing algorithms over datasets on VCU1525 platform.

| Algo. 	| rmat-21-32 | rmat-24-16  | web-google | wiki-talk| pokec | live-journal| twitter-2010|
|-------	|-------	|-------	|-------	|-------	|-------	|-------	|-------	|
| PR    	| 4,274 	| 3,797 	| 2,502 	| 3,138 	| 3,790 	| 2,860 	| 2,438 	|
| SpMV  	| 4,759  | 4,396  | 2,018  | 3,043  | 3,871  | 3,133  | 2,561  |
| BFS   	| 5,395  | 4,619  | 2,431  | 3,775  | 4,072  | 3,490  | 3,004  |
| SSSP  	| 3,895  | 3,446  | 1,817  | 2,954  | 3,090  | 2,700  | 2,273  |


Throughput (MTEPS) of different graph processing algorithms over datasets on U250 platform.

| Algo. 	| rmat-21-32 | rmat-24-16  | web-google | wiki-talk| pokec | live-journal| twitter-2010|
|-------	|-------	|-------	|-------	|-------	|-------	|-------	|-------	|
| PR    	| 4,274 	| 3,797 	| 2,502 	| 3,138 	| 3,790 	| 2,860 	| 2,438 	|
| SpMV  	| 4,759  | 4,396  | 2,018  | 3,043  | 3,871  | 3,133  | 2,561  |
| BFS   	| 5,395  | 4,619  | 2,431  | 3,775  | 4,072  | 3,490  | 3,004  |
| SSSP  	| 3,895  | 3,446  | 1,817  | 2,954  | 3,090  | 2,700  | 2,273  |

## APIs (programmability) 
Benefiting from the high level abstraction of HLS, our APIs natively support C/C++ languages.  
ThunderGraph covers three levels of API for implementation or further exploration. 
both L1 and L2 is HLS code APIs for building the FPGA accelerators, and L3 is APIs for host program.

* L1 provides the fundamental functions and basic modules used in our framework to build the compute kernels and the data flow. Users can use these API to construct their own data flow as well.

* L2 provides hooks for mapping graph processing algorithm. As we have shown our proposed data flow is efficient (refer to our paper), in this level, the data flow is fixed, and we only focus on how to map graph processing algorithm smoothly.
[Mapping new graph analytic algorithm](docs/algorithm_mapping.md)  
The figure blow show the data flow of our framework and the L2 hooks,There are two main compute unit: ***gather-scatter*** kernel and ***apply*** kernel for mapping a new algorithm, users need to handle the following hooks, calcuationUpdateValue, updateProperty etc.

* L3 provides the high level software APIs to deploy and control graph processing engine. There is a tend to using multiple-SLRs FPGA or multiple FPGAs cluster in the future heterogeneous system, therefore, L3 also wraps the partition scheduling and memory management interface for further exploration. 

   * [Memory management](docs/memory.md) 

   * [Scheduling](docs/scheduling.md) 

   * [Verification](docs/verification.md)

#### More details: [ThunderGP APIs ](docs/api_details.md)

## Related publications
* Xinyu Chen*, Ronak Bajaj^, Yao Chen, Jiong He, Bingsheng He, Weng-Fai Wong and Deming Chen. [On-The-Fly Parallel Data Shuffling for Graph Processing on OpenCL-based FPGAs](https://www.comp.nus.edu.sg/~hebs/pub/fpl19-graph.pdf). FPL, 2019.
* Xinyu Chen*, Yao Chen^, Ronak Bajaj, Jiong He, Bingsheng He, Weng-Fai Wong and Deming Chen. [Is FPGA useful for hash joins](https://www.comp.nus.edu.sg/~hebs/pub/cidr20-join.pdf). CIDR 2020: Conference on Innovative Data Systems Research


## Related systems
* Graph systems on GPU: [G3](https://github.com/Xtra-Computing/G3) | [Medusa](https://github.com/Xtra-Computing/Medusa)
* Other Thunder-series systems in Xtra NUS: [ThunderGBM](https://github.com/Xtra-Computing/thundergbm) | [ThunderSVM](https://github.com/Xtra-Computing/thundersvm)
