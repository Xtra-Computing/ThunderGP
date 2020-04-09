.PHONY: help

help::
	$(ECHO) "Makefile Usage:"
	$(ECHO) "  make all TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform>"
	$(ECHO) "      Command to generate the design for specified Target and Device."
	$(ECHO) ""
	$(ECHO) "  make clean "
	$(ECHO) "      Command to remove the generated non-hardware files."
	$(ECHO) ""
	$(ECHO) "  make cleanall"
	$(ECHO) "      Command to remove all the generated files."
	$(ECHO) ""
	$(ECHO) "  make check TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform>"
	$(ECHO) "      Command to run application in emulation."
	$(ECHO) ""

# Points to Utility Directory
COMMON_REPO = ./
ABS_COMMON_REPO = $(shell readlink -f $(COMMON_REPO))

include ./utils/utils.mk

#export  XCL_EMULATION_MODE=sw_emu
TARGETS := hw
TARGET := $(TARGETS)
DEVICES := xilinx_vcu1525_xdma_201830_1
# xilinx_vcu1525_xdma_201830_1
DEVICE := $(DEVICES)
XCLBIN := ./xclbin
DSA := $(call device2sandsa, $(DEVICE))

CXX := $(XILINX_SDX)/bin/xcpp
XOCC := $(XILINX_SDX)/bin/xocc

CXXFLAGS := $(opencl_CXXFLAGS) -Wall
LDFLAGS := $(opencl_LDFLAGS)

################################################################################

################################################################################


#--kernel_frequency <frequency>
#--xp prop:solution.kernel_compiler_margin=<Frequency Percentage>
#--xp param:compiler.enableAutoFrequencyScaling=0

HOST_SRCS = ./host_graph.cpp ./libgraph/graph.cpp ./libgraph/he_mem.cpp ./libgraph/data_helper.cpp
HOST_SRCS += ./libgraph/host_graph_sw_verification.cpp  ./libgraph/host_graph_sw.cpp

# Host compiler global settings
CXXFLAGS = -I $(XILINX_XRT)/include/ -I/$(XILINX_SDX)/Vivado_HLS/include/ -O3 -g -Wall -fmessage-length=0 -std=c++14 -Wno-deprecated-declarations
LDFLAGS = -lOpenCL -lpthread -lrt -lstdc++ -L$(XILINX_XRT)/lib/ -lxilinxopencl

CXXFLAGS += -I ./

CXXFLAGS += -I ./libfpga
CXXFLAGS += -I ./libgraph

# Kernel compiler global settings
CLFLAGS = -t $(TARGET) --platform $(DEVICE) --save-temps  -O3

CLFLAGS += -I ./libfpga

CLFLAGS += --xp prop:solution.kernel_compiler_margin=10%

# Kernel linker flags
LDCLFLAGS += --xp prop:solution.kernel_compiler_margin=10% --kernel_frequency=280

EXECUTABLE = host_graph_fpga

EMCONFIG_DIR = $(XCLBIN)/$(DSA)

BINARY_CONTAINERS += $(XCLBIN)/graph_fpga.$(TARGET).$(DSA).xclbin


#Include Libraries
include $(ABS_COMMON_REPO)/opencl/opencl.mk
include $(ABS_COMMON_REPO)/xcl/xcl.mk
CXXFLAGS +=  $(xcl_CXXFLAGS)
LDFLAGS +=   $(xcl_CXXFLAGS)
HOST_SRCS += $(xcl_SRCS)

CP = cp -rf

.PHONY: all clean cleanall docs emconfig
all: $(EXECUTABLE) $(BINARY_CONTAINERS) emconfig

.PHONY: exe
exe: cleanexe $(EXECUTABLE)

# Building kernel

$(XCLBIN)/readEdgesCU1.$(TARGET).$(DSA).xo: ./graph_fpga_cu1.cpp
	mkdir -p $(XCLBIN)
	$(XOCC) $(CLFLAGS) -c -k readEdgesCU1 -I'$(<D)' -o'$@' '$<'
BINARY_CONTAINER_OBJS += $(XCLBIN)/readEdgesCU1.$(TARGET).$(DSA).xo
BINARY_LINK_OBJS    += --nk  readEdgesCU1:1
BINARY_LINK_OBJS    += --sp  readEdgesCU1_1.edgeScoreMap:DDR[3]
BINARY_LINK_OBJS    += --sp  readEdgesCU1_1.edges:DDR[3]
BINARY_LINK_OBJS    += --sp  readEdgesCU1_1.vertexScore:DDR[3]
BINARY_LINK_OBJS    += --sp  readEdgesCU1_1.tmpVertexProp:DDR[3]
BINARY_LINK_OBJS    += --slr readEdgesCU1_1:SLR2



$(XCLBIN)/readEdgesCU2.$(TARGET).$(DSA).xo: ./graph_fpga_cu2.cpp
	mkdir -p $(XCLBIN)
	$(XOCC) $(CLFLAGS) -c -k readEdgesCU2 -I'$(<D)' -o'$@' '$<'
BINARY_CONTAINER_OBJS += $(XCLBIN)/readEdgesCU2.$(TARGET).$(DSA).xo
BINARY_LINK_OBJS    += --nk  readEdgesCU2:1
BINARY_LINK_OBJS    += --sp  readEdgesCU2_1.edgeScoreMap:DDR[2]
BINARY_LINK_OBJS    += --sp  readEdgesCU2_1.edges:DDR[2]
BINARY_LINK_OBJS    += --sp  readEdgesCU2_1.vertexScore:DDR[2]
BINARY_LINK_OBJS    += --sp  readEdgesCU2_1.tmpVertexProp:DDR[2]
BINARY_LINK_OBJS    += --slr readEdgesCU2_1:SLR2



$(XCLBIN)/readEdgesCU3.$(TARGET).$(DSA).xo: ./graph_fpga_cu3.cpp
	mkdir -p $(XCLBIN)
	$(XOCC) $(CLFLAGS) -c -k readEdgesCU3 -I'$(<D)' -o'$@' '$<'
BINARY_CONTAINER_OBJS += $(XCLBIN)/readEdgesCU3.$(TARGET).$(DSA).xo
BINARY_LINK_OBJS    += --nk  readEdgesCU3:1
BINARY_LINK_OBJS    += --sp  readEdgesCU3_1.edgeScoreMap:DDR[1]
BINARY_LINK_OBJS    += --sp  readEdgesCU3_1.edges:DDR[1]
BINARY_LINK_OBJS    += --sp  readEdgesCU3_1.vertexScore:DDR[1]
BINARY_LINK_OBJS    += --sp  readEdgesCU3_1.tmpVertexProp:DDR[1]
BINARY_LINK_OBJS    += --slr readEdgesCU3_1:SLR0



$(XCLBIN)/readEdgesCU4.$(TARGET).$(DSA).xo: ./graph_fpga_cu4.cpp
	mkdir -p $(XCLBIN)
	$(XOCC) $(CLFLAGS) -c -k readEdgesCU4 -I'$(<D)' -o'$@' '$<'
BINARY_CONTAINER_OBJS += $(XCLBIN)/readEdgesCU4.$(TARGET).$(DSA).xo
BINARY_LINK_OBJS    += --nk  readEdgesCU4:1
BINARY_LINK_OBJS    += --sp  readEdgesCU4_1.edgeScoreMap:DDR[0]
BINARY_LINK_OBJS    += --sp  readEdgesCU4_1.edges:DDR[0]
BINARY_LINK_OBJS    += --sp  readEdgesCU4_1.vertexScore:DDR[0]
BINARY_LINK_OBJS    += --sp  readEdgesCU4_1.tmpVertexProp:DDR[0]
BINARY_LINK_OBJS    += --slr readEdgesCU4_1:SLR0


$(XCLBIN)/vertexApply.$(TARGET).$(DSA).xo: ./graph_fpga_vertexApply.cpp
	mkdir -p $(XCLBIN)
	$(XOCC) $(CLFLAGS) -c -k vertexApply -I'$(<D)' -o'$@' '$<'
BINARY_CONTAINER_OBJS += $(XCLBIN)/vertexApply.$(TARGET).$(DSA).xo
BINARY_LINK_OBJS    += --nk  vertexApply:1
BINARY_LINK_OBJS    += --sp  vertexApply_1.vertexProp:DDR[1]


BINARY_LINK_OBJS    += --sp  vertexApply_1.outDegree:DDR[2]

BINARY_LINK_OBJS    += --sp  vertexApply_1.newVertexProp1:DDR[1]
BINARY_LINK_OBJS    += --sp  vertexApply_1.tmpVertexProp1:DDR[1]

BINARY_LINK_OBJS    += --sp  vertexApply_1.newVertexProp2:DDR[2]
BINARY_LINK_OBJS    += --sp  vertexApply_1.tmpVertexProp2:DDR[2]

BINARY_LINK_OBJS    += --sp  vertexApply_1.newVertexProp3:DDR[3]
BINARY_LINK_OBJS    += --sp  vertexApply_1.tmpVertexProp3:DDR[3]

BINARY_LINK_OBJS    += --sp  vertexApply_1.newVertexProp4:DDR[0]
BINARY_LINK_OBJS    += --sp  vertexApply_1.tmpVertexProp4:DDR[0]

BINARY_LINK_OBJS    += --sp  vertexApply_1.error:DDR[2]
BINARY_LINK_OBJS    += --slr vertexApply_1:SLR1

$(XCLBIN)/graph_fpga.$(TARGET).$(DSA).xclbin: $(BINARY_CONTAINER_OBJS)
	$(XOCC) $(CLFLAGS) -l $(LDCLFLAGS)   $(BINARY_LINK_OBJS) -o'$@' $(+)

# Building Host
$(EXECUTABLE): $(HOST_SRCS)
	mkdir -p $(XCLBIN)
	$(CXX) $(CXXFLAGS) $(HOST_SRCS) -o '$@' $(LDFLAGS)

emconfig:$(EMCONFIG_DIR)/emconfig.json
$(EMCONFIG_DIR)/emconfig.json:
	emconfigutil --platform $(DEVICE) --od $(EMCONFIG_DIR)

check: all

ifeq ($(TARGET),$(filter $(TARGET),sw_emu hw_emu))
	$(CP) $(EMCONFIG_DIR)/emconfig.json .
	XCL_EMULATION_MODE=$(TARGET) ./$(EXECUTABLE) 
else
	 ./$(EXECUTABLE)
endif
	sdx_analyze profile -i sdaccel_profile_summary.csv -f html

# Cleaning stuff

cleanexe:
	-$(RMDIR) $(EXECUTABLE)
clean:
	-$(RMDIR) $(EXECUTABLE) $(XCLBIN)/{*sw_emu*,*hw_emu*} 
	-$(RMDIR) sdaccel_* TempConfig system_estimate.xtxt *.rpt
	-$(RMDIR) src/*.ll _xocc_* .Xil emconfig.json dltmp* xmltmp* *.log *.jou *.wcfg *.wdb
	-$(RMDIR) .Xil
	-$(RMDIR) *.zip
	-$(RMDIR) *.str

cleanall: clean
	-$(RMDIR) $(XCLBIN)
	-$(RMDIR) ./_x
	-$(RMDIR) ./membership.out


