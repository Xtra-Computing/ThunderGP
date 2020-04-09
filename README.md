# On-the-fly Data Shuffling for OpenCL-based FPGAs

## Prerequisites
* The gcc-4.8 or above
* The Altera OpenCL SDK 16.0.2 for FPGA 
* The De5net board 

## Run the code

Do not forget to set the PATH of the dataset. 

```sh
$ cd ./
$ make  # make the host execution program
$ aoc ./src/graph_fpga.cl -o ./bin/graph_fpga.aocx  # make the FPGA execution program. It takes time.
$ cd ./bin
$ ./host
```
