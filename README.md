[![GitHub license](https://img.shields.io/badge/license-apache2-yellowgreen)](./LICENSE)
[![GitHub issues](https://img.shields.io/github/issues/Xtra-Computing/On-the-fly-data-shuffling-for-OpenCL-based-FPGAs.svg)](https://github.com/Xtra-Computing/On-the-fly-data-shuffling-for-OpenCL-based-FPGAs/issues)
## Prerequisites
* The gcc-4.8 or above
* The SDAccel 2018.3 Design Suit
* The Xilinx Virtex UltraScale+ FPGA VCU1525 Acceleration Development Kit

## Run the code

Do not forget to set the PATH of the dataset. 

```sh
$ cd ./
$ make cleanall
$ make app=pr all -j # make the host execution program and FPGA execution program for pagerank application. It takes time.
$ ./host [bitfile] [graph name] #e.g., ./host_graph_fpga _x/link/int/graph_fpga.hw.xilinx_vcu1525_xdma_201830_1.xclbin wiki-talk
```
Currently, our framework support accelerate four graph analytic algorithm, it can be selected by compile parameter ```app=xxx``` in the make command, bellowing table is for quick reference of this parameter



| Parameter    | Accelerated algorithm  |
|--------------|--------------|
| ```app=pr``` | PageRank |
| ```app=spmv``` | Sparse matrix-vector multiplication (SpMV) |
| ```app=bfs``` | Breadth first search |
| ```app=sssp``` | Single Source Shortest Path |

Details:
[Compile ThunderGP ](docs/compile_arch.md)


## API Definition
ThunderGraph covers three levels of API for implementation or further exploration. 
### Overview
both L1 and L2 is HLS code APIs for building the FPGA accelerators, and L3 is APIs for host program.
#### L1
L1 provides the fundamental functions and basic modules used in our framework to build the compute kernels and the data flow. Users can use these API to construct their own data flow as well.

#### L2
L2 provides hooks for mapping graph processing algorithm. As we have shown our proposed data flow is efficient (refer to our paper), in this level, the data flow is fixed, and we only focus on how to map graph processing algorithm smoothly.

[Mapping new graph analytic algorithm](docs/algorithm_mapping.md)

The figure blow show the data flow of our framework and the L2 hooks,There are two main compute unit: ***gather-scatter*** kernel and ***apply*** kernel for mapping a new algorithm, users need to handle the following hooks, calcuationUpdateValue, updateProperty etc.

#### L3
L3 provides the high level software APIs to deploy and control graph processing engine. There is a tend to using multiple-SLRs FPGA or multiple FPGAs cluster in the future heterogeneous system, therefore, L3 also wraps the partition scheduling and memory management interface for further exploration. 

[1.Memory management](docs/memory.md) 

[2.Scheduling](docs/scheduling.md) 

[3.Verification](docs/verification.md)

####Details
[ThunderGP APIs ](docs/api_details.md)




## Related publications
* Xinyu Chen*, Ronak Bajaj^, Yao Chen, Jiong He, Bingsheng He, Weng-Fai Wong and Deming Chen. [Is FPGA useful for hash joins](https://www.comp.nus.edu.sg/~hebs/pub/cidr20-join.pdf). CIDR 2020: Conference on Innovative Data Systems Research


## Related systems

* Graph systems on GPU: [G3](https://github.com/Xtra-Computing/G3) | [Medusa](https://github.com/Xtra-Computing/Medusa)
* Other Thunder-series systems in Xtra NUS: [ThunderGBM](https://github.com/Xtra-Computing/thundergbm) | [ThunderSVM](https://github.com/Xtra-Computing/thundersvm)