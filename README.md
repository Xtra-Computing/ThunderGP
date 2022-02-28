## Introduction

ThunderGP enables data scientists to enjoy the ***performance*** of FPGA-based graph processing without compromising ***programmability***. ***To our best knowledge and experiments, this is the fastest graph processing framework on HLS-based FPGAs.***

## Prerequisites
* The gcc-9.3
* Tools:
    * Vitis 2020.2 Design Suit
* Evaluated platforms from Xilinx:
    * Alveo U280 Data Center Accelerator Card
    
## Run the code
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

#### Here is the example of implementing the accelerator for PageRank on Alveo U250 platform with SDAccel 2019.1. 
```sh
$ git clone https://github.com/Xtra-Computing/ThunderGP.git
$ cd ./
$ vim ThunderGP.mk 
$ # configure the DEVICE as DEVICES := xilinx_u280_xdma_201830_2; configure TARGETS := hw
$ make app=pr all # make the host execution program and the FPGA bitstream. It takes time :)
# For execution on real hardware. The path of graph dataset needs to be provided by the user. 
$ ./host_graph_fpga_pr xclbin_pr/*.xclbin ./dataset/rmat-14-32.txt
```
#### More details: [Compiling ThunderGP ](docs/compile_arch.md)


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

## How to cite **ThunderGP** 
If you use **ThunderGP**  in your paper, please cite our work ([full version](https://github.com/Xtra-Computing/ThunderGP/blob/master/ThunderGP_camera_ready-pdfa.pdf)).
```
@inbook{10.1145/3431920.3439290,
author = {Chen, Xinyu and Tan, Hongshi and Chen, Yao and He, Bingsheng and Wong, Weng-Fai and Chen, Deming},
title = {ThunderGP: HLS-Based Graph Processing Framework on FPGAs},
year = {2021},
url = {https://doi.org/10.1145/3431920.3439290},
booktitle = {The 2021 ACM/SIGDA International Symposium on Field-Programmable Gate Arrays},
pages = {69–80},
numpages = {12}
}
```


