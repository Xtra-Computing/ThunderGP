include ThunderGP.mk

CODE_GEN_PATH   =./automation
CODE_GEN_PARSER_PATH  = $(CODE_GEN_PATH)/parser
CODE_GEN_FILE =  $(CODE_GEN_PATH)/parser.cpp
CODE_GEN_FILE += $(CODE_GEN_PATH)/parser_debug.cpp
CODE_GEN_FILE += $(CODE_GEN_PARSER_PATH)/mem_interface.cpp
CODE_GEN_FILE += $(CODE_GEN_PARSER_PATH)/kernel_interface.cpp
CODE_GEN_FILE += $(CODE_GEN_PARSER_PATH)/customize.cpp
CODE_GEN_FILE += $(CODE_GEN_PARSER_PATH)/makefile.cpp


# AUTOGEN_CFLAG in here:
include tmp_para/para.mk


INCLUDE_FLAG =  -I application
INCLUDE_FLAG += -I libgraph
INCLUDE_FLAG += -I $(CODE_GEN_PATH)
INCLUDE_FLAG += -I $(CODE_GEN_PARSER_PATH)


VAR_TRUE=true
APP = $(app)
APPCONFIG = ./application/$(APP)
include $(APPCONFIG)/build.mk

.PHONY: code_gen
code_gen:

	rm -rf tmp_fpga_top
	mkdir -p tmp_fpga_top
	g++  -static-libstdc++ $(INCLUDE_FLAG) $(AUTOGEN_CFLAG) $(CODE_GEN_FILE) -o code_gen
	./code_gen libfpga/common_template/apply_top.cpp             tmp_fpga_top/apply_top
	./code_gen libfpga/common_template/scatter_gather_top.cpp    tmp_fpga_top/scatter_gather_top
ifndef app
	$(error app is undefined)
else
ifeq ($(strip $(CUSTOMIZE_APPLY)), $(strip $(VAR_TRUE)))
	./code_gen libfpga/customize_template/customize_apply_top.cpp        application/$(app)/customized_apply.cpp  tmp_fpga_top/customize_apply_top
	./code_gen libfpga/customize_template/customize_mem.h                application/$(app)/customized_apply.cpp  tmp_fpga_top/customize_mem
	./code_gen libfpga/customize_template/customize_apply_cl_kernel.h    application/$(app)/customized_apply.cpp  tmp_fpga_top/customize_apply_cl_kernel
endif
endif
