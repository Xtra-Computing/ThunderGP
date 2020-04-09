
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
