include ThunderGP.mk

CODE_GEN_PATH  =./libfpga/generator
CODE_GEN_FILE  =  $(CODE_GEN_PATH)/makefile_gen.cpp
CODE_GEN_FILE += -I libgraph/
# AUTOGEN_CFLAG in here:
include tmp_para/para.mk

.PHONY: makefile_gen
makefile_gen:
	rm -f tmp_fpga_top/*.mk
	mkdir -p tmp_fpga_top
	g++  -static-libstdc++ -I application $(AUTOGEN_CFLAG) $(CODE_GEN_FILE) -o makefile_gen
	./makefile_gen 



