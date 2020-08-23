
SHELL := /bin/bash
COMMON_REPO = ./
ABS_COMMON_REPO = $(shell readlink -f $(COMMON_REPO))
UTILS_PATH = ./utils

include $(UTILS_PATH)/help.mk
include $(UTILS_PATH)/utils.mk

TARGETS := hw
TARGET  := $(TARGETS)
DEVICES := xilinx_u200_xdma_201830_2
# device list:
# xilinx_vcu1525_xdma_201830_1
# xilinx_u200_xdma_201830_2
# xilinx_u250_xdma_201830_2

DEVICE  := $(DEVICES)

APP = $(app)
APPCONFIG = ./application/$(APP)

include $(APPCONFIG)/config.mk
include $(APPCONFIG)/build.mk
include ./application/common.mk


CODE_GEN_PATH=./libfpga/generator
include $(CODE_GEN_PATH)/parser.mk

.PHONY:application
application:: $(APPCONFIG)
	@touch $(APPCONFIG)/build.mk
	@echo $(opencl_LDFLAGS)

.PHONY: all clean cleanall emconfig
all: code_gen $(EXECUTABLE) $(BINARY_CONTAINERS) emconfig application

.PHONY: exe
exe: cleanexe $(EXECUTABLE)


include ./application/common_gs_kernel.mk
include ./application/common_apply_kernel.mk


$(XCLBIN)/graph_fpga.$(TARGET).$(DSA).xclbin: $(BINARY_CONTAINER_OBJS)
	$(XOCC) $(CLFLAGS) -l $(LDCLFLAGS)   $(BINARY_LINK_OBJS) -o'$@' $(+)

# Building Host
$(EXECUTABLE): $(HOST_SRCS)
	mkdir -p $(XCLBIN)
	$(CXX) $(CXXFLAGS) $(HOST_SRCS) -o '$@' $(LDFLAGS)

emconfig:$(EMCONFIG_DIR)/emconfig.json
$(EMCONFIG_DIR)/emconfig.json:
	emconfigutil --platform $(DEVICE) --od $(EMCONFIG_DIR)

.PHONY: hwemuprepare
hwemuprepare:
ifeq ($(TARGET),$(filter $(TARGET), hw_emu))
	@echo "prepare for hw_emu"
	$(CP) $(EMCONFIG_DIR)/emconfig.json .
	$(CP) $(UTILS_PATH)/sdaccel.ini .
	source $(UTILS_PATH)/hw_emu.sh
else
	@echo "prepare for hw"
endif


check: all 

ifeq ($(TARGET),$(filter $(TARGET),sw_emu hw_emu))
	$(CP) $(EMCONFIG_DIR)/emconfig.json .
	XCL_EMULATION_MODE=$(TARGET) ./$(EXECUTABLE) 
else
	 ./$(EXECUTABLE)
endif
	sdx_analyze profile -i sdaccel_profile_summary.csv -f html



cleanexe:
	-$(RMDIR) $(EXECUTABLE)
clean:
	-$(RMDIR) $(EXECUTABLE) $(XCLBIN)/{*sw_emu*,*hw_emu*} 
	-$(RMDIR) sdaccel_* TempConfig system_estimate.xtxt *.rpt
	-$(RMDIR) src/*.ll _xocc_* .Xil emconfig.json dltmp* xmltmp* *.log *.jou *.wcfg *.wdb
	-$(RMDIR) .Xil
	-$(RMDIR) *.zip
	-$(RMDIR) *.str
	-$(RMDIR) $(XCLBIN)
	-$(RMDIR) ./_x
	-$(RMDIR) ./membership.out
	-$(RMDIR) host_graph_fpga*
	-$(RMDIR) xclbin*
	-$(RMDIR) .run
	-$(RMDIR) tmp_fpga_top


