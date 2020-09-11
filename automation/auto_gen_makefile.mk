include ThunderGP.mk

CODE_GEN_PATH  =./automation
CODE_GEN_FILE  =  $(CODE_GEN_PATH)/makefile_gen.cpp
CODE_GEN_FILE += -I libgraph/
# AUTOGEN_CFLAG in here:
include tmp_para/para.mk


VAR_TRUE=true
APP = $(app)
APPCONFIG = ./application/$(APP)
include $(APPCONFIG)/build.mk

.PHONY: makefile_gen
makefile_gen:
	rm -f tmp_fpga_top/*.mk
	mkdir -p tmp_fpga_top
	g++  -static-libstdc++ -I application $(AUTOGEN_CFLAG) $(CODE_GEN_FILE) -o makefile_gen
	./makefile_gen
	
ifndef app
	$(error app is undefined)
else
ifeq ($(strip $(CUSTOMIZE_APPLY)), $(strip $(VAR_TRUE)))
	./code_gen libfpga/customize_template/customize_apply_kernel.mk   application/$(app)/customized_apply.cpp tmp_fpga_top/apply_kernel
else
	./code_gen libfpga/common_template/apply_kernel.mk                tmp_fpga_top/apply_kernel
endif
endif
