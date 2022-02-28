## Introduction

ThunderGP for HBM-Enabled platforms. 

## Prerequisites
* The gcc-9.3
* Tools:
    * Xilinx Vitis 2020.2 Design Suit
* Evaluated platforms from Xilinx:
    * Xilinx Alveo U280 Data Center Accelerator Card
    * Xilinx Alveo U50 Data Center Accelerator Card

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
$ git checkout -b v_HBM
$ vim ThunderGP.mk 
$ # configure the DEVICE as DEVICES := xilinx_u280_xdma_201920_3; configure TARGETS := hw
$ make app=pr all # make the host execution program and the FPGA bitstream. It takes time :)
# For execution on real hardware. The path of graph dataset needs to be provided by the user. 
$ ./host_graph_fpga_pr xclbin_pr/*.xclbin ./dataset/rmat-14-32.txt
```




