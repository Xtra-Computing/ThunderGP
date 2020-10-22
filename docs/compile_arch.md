# Compiling ThunderGP
This page provides details of quickly deploying build-in graph analytic algorithms of ThunderGP.  
The compilation of ThunderGP is managed by [GNU make](https://www.gnu.org/software/make/manual/html_node/Introduction.html).

## Compilation Arguments

Currently, ThunderGP supports four graph analytic applications, namely PR, SpMV, BFS and SSSP. The wanted application can be implemented by passing argument app=[the wanted application] to the make command. The below table are details of this argument.

| Argument    | Accelerated algorithm  |
|--------------|--------------|
| ```app=pr``` | PageRank |
| ```app=spmv``` | Sparse matrix-vector multiplication (SpMV) |
| ```app=bfs``` | Breadth first search |
| ```app=sssp``` | Single Source Shortest Path |

Other arguments the developers may use. 

| Argument    | Description |
|--------------|--------------|
| ```all``` | Compile host + cccelerator programs, it is time-costly (10+ hours) |
| ```exe``` | Compile host program only, it is very fast |
| ```cleanexe``` | Clean the host program |
| ```clean``` | Clean the accelerator program |
| ```cleanall``` | Clean all of the host + accelerator programs |


For example, if you want to implement the PageRank, the command is:

```sh
$ make app=pr all
```

## Makefile Tree

The Makefile file structure is shown in the bellowing tree:

```sh
├── application
│   ├── common.mk    # general configuration for both host and accelerator programs
│   ├── gs_kernel.mk # accelerator configuration for gather-scatter kernel 
│   ├── pr                    # application specific folder
│   │   ├── apply_kernel.mk   # accelerator configuration for apply kernel
│   │   ├── build.mk          # application specific build configuration
│   │   ├── config.mk         # application specific design parameters
│   └── ...
├── docs
├── libfpga
├── libgraph
├── Makefile  # Main entrance for make
├── README.md
├── utils
│   ├── help.mk      # Help information
│   ├── opencl.mk    # OpenCL library
│   └── utils.mk     # Misc.
└── xcl
    └── xcl.mk       # Xilinx OpenCL library
```

## Fast Debugging Compilation

Compiling the entire accelerator can be very time-costly (*__14+ hours__*), and it is very unfriendly for iteratively debugging. ThunderGP provides a simplified and fast mode for compilation by only utilizing one SLR, it also means that in this mode, only have one __Scatter-Gather CU__ and the compilation time can be significantly reduced to about *__4 hours__*. 

To enable fast compilation for debugging, you need to change the ```HAVE_FULL_SLR``` to false in the ```build.mk``` located at the application-specific directory, then start the build. The host program do not need any manual change, as it will automatically adapt partitioning according to this configuration.


## Waveform-based Debugging


Xilinx provide real-time waveform display in __hw_emu__ mode, but it need many setting-up steps in command-line. ThunderGP simplified the details to use this feature.

* Change the ```TARGETS``` to ```hw_emu``` in main Makefile, or directly pass this argument from the make command.
* After the application has been built, run ```make hwemuprepare```
* Run env setup script:```source ./utils/hw_emu.sh ```
* Start the program

__Notes__:
We found that the waveform debugging in SDAccel 2019.2 have one problem: all of the stream interfaces are not displayed correctly. Therefore we only use SDAccel 2018.3 to perform the waveform-based debugging.



## Timing Problems

As ThunderGP has a very high resources utilization, the timing issues can be very difficult to be fixed from high-level-synthesis code. our framework used the stream slicing technique and multi-level data duplication technique to achieve a higher frequency(__250MHz__). the details can be found in our paper. However, there are still many randomized factors in the placement and routine stage which can not be controlled by our HLS code. From our experience, we always start __3__ same compilation, which could result with a better frequency finally.  
