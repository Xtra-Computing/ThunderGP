
## Prerequisites
* The gcc-4.8 or above
* The SDAccel 2018.3 Design Suit
* The Xilinx Virtex UltraScale+ FPGA VCU1525 Acceleration Development Kit

## Run the code

Do not forget to set the PATH of the dataset. 

```sh
$ cd ./
$ make clean all
$ make all -g # make the host execution program and FPGA execution program. It takes time.
$ ./host [bitfile] [graph name]
```
