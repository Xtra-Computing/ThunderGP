### Memory management
Currently, Xilinx Multiple-SLRs FPGAs have many independent DRAM banks, and many graph algorithms manipulate more than five types of data. Managing the data and the partitions among the banks using the traditional OpenCL APIs need a lot of redundant code for configuration, which makes the code hard to maintaining, Therefore we developed an unique ID based memory management module, by using this module the algorithm will not need to aware the physical layout among DRAM banks, it also provide well scalability for Multiple-SLRs FPGAs.


| Function    | Description  |
|-------------|--------------|
| ``` int register_size_attribute() ```| |
| ``` unsigned int get_size_attribute() ```| |
| ``` int he_mem_init() ```| |
| ``` cl_mem* get_cl_mem_pointer() ```| |
| ``` void* get_host_mem_pointer() ```| |
| ``` he_mem_t* get_he_mem() ```| |
| ``` void clear_host_mem() ```| |
| ``` void hardware_init() ```| |
| ``` int transfer_data_from_pl() ```| |
| ``` int transfer_data_to_pl() ```| |
