## Compiling ThunderGP
This page provides details of quickly deploying build-in graph analytic algorithms of ThunderGP.  
The compilation of ThunderGP is managed by [GNU make](https://www.gnu.org/software/make/manual/html_node/Introduction.html).

### Compilation Arguments

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
$ make app=pr all -j
```

#### Makefile tree

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

#### Application-specific compilation
As mentioned, ThunderGP supports four graph analytic algorithms. Currently, the difference among algorithms is uniformed by architecture-based configurations which are abstracted by ourself, and these configurations can be incremental added to ThunderGP for support new algorithms in the future.

The application-specific configuration is located in the ```application``` folder, and each algorithm has a sub-folder in this path, The corresponding configurations are stored in this sub-folder.

The following table shows the existing application-specific configurations for compilation (the ```build.mk```)


| Configuration | Value | Description  |
|---------------|-------|--------------|
| HAVE_FULL_SLR |          true/false  | single SLR or multiple SLRs   |
| HAVE_APPLY    |          true/false  | have apply kernel or not   |
| HAVE_VERTEX_ACTIVE_BIT | true/false  | the vertex property is masked or not    |
| HAVE_EDGE_PROP |         true/false  | the tuple have edge property or not   |
| HAVE_UNSIGNED_PROP    |  true/false  | the type of property is unsigned or not   |

Details:

[Mapping new graph analytic algorithm](docs/algorithm_mapping.md)
