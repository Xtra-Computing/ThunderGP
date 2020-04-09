
## Prerequisites
* The gcc-4.8 or above
* The SDAccel 2018.3 Design Suit
* The Xilinx Virtex UltraScale+ FPGA VCU1525 Acceleration Development Kit

## Run the code

Do not forget to set the PATH of the dataset. 

```sh
$ cd ./
$ make all # make the host execution program
$ aoc ./src/graph_fpga.cl -o ./bin/graph_fpga.aocx  # make the FPGA execution program. It takes time.
$ cd ./bin
$ ./host
```
