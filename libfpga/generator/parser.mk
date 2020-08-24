

CODE_GEN_FILE =  $(CODE_GEN_PATH)/parser.cpp
CODE_GEN_FILE += $(CODE_GEN_PATH)/mem_interface.cpp
CODE_GEN_FILE += $(CODE_GEN_PATH)/kernel_interface.cpp
CODE_GEN_FILE += $(CODE_GEN_PATH)/parser_debug.cpp

.PHONY: code_gen
code_gen:
	rm -rf tmp_fpga_top
	mkdir -p tmp_fpga_top
	g++  -static-libstdc++ $(GENFLAGS) $(CODE_GEN_FILE) -o code_gen
	./code_gen libfpga/common/apply_top.cpp             tmp_fpga_top/apply_top
	./code_gen libfpga/common/scatter_gather_top.cpp    tmp_fpga_top/scatter_gather_top

