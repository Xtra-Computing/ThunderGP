#!/bin/sh

date_str=`date +%Y%m%d%T`
log_path=test_log_${date_str}
mkdir -p ${log_path}

./host_graph_fpga _x/link/int/graph_fpga.hw.xilinx_vcu1525_xdma_201830_1.xclbin rmat-19-32   > ./${log_path}/r19.log
./host_graph_fpga _x/link/int/graph_fpga.hw.xilinx_vcu1525_xdma_201830_1.xclbin rmat-21-32   > ./${log_path}/r21.log
./host_graph_fpga _x/link/int/graph_fpga.hw.xilinx_vcu1525_xdma_201830_1.xclbin rmat-24-16   > ./${log_path}/r24.log

./host_graph_fpga _x/link/int/graph_fpga.hw.xilinx_vcu1525_xdma_201830_1.xclbin mouse-gene   > ./${log_path}/mg.log
./host_graph_fpga _x/link/int/graph_fpga.hw.xilinx_vcu1525_xdma_201830_1.xclbin pokec        > ./${log_path}/pk.log
./host_graph_fpga _x/link/int/graph_fpga.hw.xilinx_vcu1525_xdma_201830_1.xclbin google       > ./${log_path}/gg.log
./host_graph_fpga _x/link/int/graph_fpga.hw.xilinx_vcu1525_xdma_201830_1.xclbin wiki-talk    > ./${log_path}/wt.log
./host_graph_fpga _x/link/int/graph_fpga.hw.xilinx_vcu1525_xdma_201830_1.xclbin lj1          > ./${log_path}/lj.log
./host_graph_fpga _x/link/int/graph_fpga.hw.xilinx_vcu1525_xdma_201830_1.xclbin twitter-2010 > ./${log_path}/tw.log

