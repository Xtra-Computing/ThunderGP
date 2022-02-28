VAR_TRUE=true

CP = cp -rf
XCLBIN := ./xclbin_$(TARGET)_$(APP)
DSA := $(call device2sandsa, $(DEVICE))

HOST_ARCH := x86

#Checks for g++
ifeq ($(HOST_ARCH), x86)
ifneq ($(shell expr $(shell g++ -dumpversion) \>= 5), 1)
ifndef XILINX_VIVADO
$(error [ERROR]: g++ version older. Please use 5.0 or above.)
else
CXX := $(XILINX_VIVADO)/tps/lnx64/gcc-6.2.0/bin/g++
$(warning [WARNING]: g++ version older. Using g++ provided by the tool : $(CXX))
endif
endif
else ifeq ($(HOST_ARCH), aarch64)
CXX := $(XILINX_VITIS)/gnu/aarch64/lin/aarch64-linux/bin/aarch64-linux-gnu-g++
else ifeq ($(HOST_ARCH), aarch32)
CXX := $(XILINX_VITIS)/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin/arm-linux-gnueabihf-g++
endif

XOCC := v++

GS_KERNEL_PATH    = ./acc_top
APPLY_KERNEL_PATH = ./acc_top

include $(ABS_COMMON_REPO)/utils/opencl.mk

#--kernel_frequency <frequency>
#--xp prop:solution.kernel_compiler_margin=<Frequency Percentage>
#--xp param:compiler.enableAutoFrequencyScaling=0

HOST_SRCS = ./libgraph/misc/graph.cpp ./libgraph/misc/data_helper.cpp

ifeq ($(strip $(DEFAULT_ENTRY)), $(strip $(VAR_TRUE)))
	HOST_SRCS +=  ./app_udfs/default_main.cpp
else
	HOST_SRCS +=  $(APPCONFIG)/main.cpp
endif

HOST_SRCS += ./libgraph/memory/he_mem.cpp
HOST_SRCS += ./libgraph/memory/he_mapping.cpp
HOST_SRCS += ./libgraph/misc/host_graph_mem.cpp
HOST_SRCS += ./libgraph/host_graph_partition.cpp
HOST_SRCS += ./libgraph/kernel/host_graph_kernel.cpp
HOST_SRCS += ./libgraph/host_graph_dataflow.cpp
HOST_SRCS += ./libgraph/scheduler/host_graph_scheduler.cpp
HOST_SRCS += ./libgraph/scheduler/$(SCHEDULER)/scheduler.cpp
HOST_SRCS += $(APPCONFIG)/dataPrepare.cpp
HOST_SRCS += ./libgraph/verification/host_graph_verification_gs.cpp
HOST_SRCS += ./libgraph/verification/host_graph_cmodel.cpp

ifeq ($(strip $(HAVE_APPLY)), $(strip $(VAR_TRUE)))
    ifeq ($(strip $(CUSTOMIZE_APPLY)), $(strip $(VAR_TRUE)))
        HOST_SRCS += $(APPCONFIG)/host_vertex_apply.cpp
    endif
        HOST_SRCS += ./libgraph/verification/host_graph_verification_apply.cpp
endif


AUTOGEN_CFLAG =  -DSUB_PARTITION_NUM=$(SUB_PARTITION_NUM)
AUTOGEN_CFLAG += -DPARTITION_SIZE=$(PARTITION_SIZE)

# Host compiler global settings
CXXFLAGS :=  $(AUTOGEN_CFLAG) 

CXXFLAGS += $(opencl_CXXFLAGS) -Wall
CXXFLAGS += -I/$(XILINX_SDX)/Vivado_HLS/include/ -O3 -g -fmessage-length=0 -std=c++14 -Wno-deprecated-declarations
CXXFLAGS += -I ./
CXXFLAGS += -I ./libfpga
CXXFLAGS += -I ./libgraph
CXXFLAGS += -I ./libgraph/memory
CXXFLAGS += -I ./libgraph/scheduler
CXXFLAGS += -I ./libgraph/verification
CXXFLAGS += -I ./libgraph/misc
CXXFLAGS += -I ./libgraph/kernel
CXXFLAGS += -I $(APPCONFIG)
CXXFLAGS += -I ./app_udfs
# CXXFLAGS += -I acc_para
CXXFLAGS += -I acc_top

# Host linker flags
LDFLAGS := $(opencl_LDFLAGS)
LDFLAGS += -lrt -lstdc++  -lxilinxopencl


CLFLAGS := $(AUTOGEN_CFLAG)

ifeq ($(TARGET),$(filter $(TARGET), hw_emu))
CLFLAGS += -g -t $(TARGET)
else
CLFLAGS += -t $(TARGET)
endif

# Kernel compiler global settings
CLFLAGS += --platform $(DEVICE) --save-temps  -O3
CLFLAGS += -I ./
CLFLAGS += -I ./libfpga
CLFLAGS += -I ./libfpga/common
CLFLAGS += -I $(APPCONFIG)
CLFLAGS += -I ./app_udfs
CLFLAGS += --xp prop:solution.kernel_compiler_margin=10%

# Kernel linker flags

LDCLFLAGS += --xp prop:solution.kernel_compiler_margin=10% 
#--kernel_frequency=$(FREQ)

EXECUTABLE = host_graph_fpga_$(APP)

EMCONFIG_DIR = $(XCLBIN)/$(DSA)

BINARY_CONTAINERS += $(XCLBIN)/graph_fpga.$(TARGET).$(DSA).xclbin


#Include Libraries

include $(UTILS_PATH)/xcl/xcl.mk
CXXFLAGS +=  $(xcl_CXXFLAGS)
LDFLAGS +=   $(xcl_CXXFLAGS)
HOST_SRCS += $(xcl_SRCS)


#############################################################################
#                                                                           #
#                     Specific Build Configuration                          #
#                                                                           #
#############################################################################


ifeq ($(strip $(HAVE_APPLY)), $(strip $(VAR_TRUE)))
CXXFLAGS += -DHAVE_APPLY=1
else
CXXFLAGS += -DHAVE_APPLY=0
endif


ifeq ($(strip $(HAVE_EDGE_PROP)), $(strip $(VAR_TRUE)))
CXXFLAGS += -DHAVE_EDGE_PROP=1
CLFLAGS  += -DHAVE_EDGE_PROP=1
else
CXXFLAGS += -DHAVE_EDGE_PROP=0
CLFLAGS  += -DHAVE_EDGE_PROP=0
endif


ifeq ($(strip $(HAVE_UNSIGNED_PROP)), $(strip $(VAR_TRUE)))
CXXFLAGS += -DHAVE_UNSIGNED_PROP=1
CLFLAGS  += -DHAVE_UNSIGNED_PROP=1
else
CXXFLAGS += -DHAVE_UNSIGNED_PROP=0
CLFLAGS  += -DHAVE_UNSIGNED_PROP=0
endif


ifeq ($(strip $(HAVE_APPLY_OUTDEG)), $(strip $(VAR_TRUE)))
CXXFLAGS += -DHAVE_APPLY_OUTDEG=1
CLFLAGS  += -DHAVE_APPLY_OUTDEG=1
else
CXXFLAGS += -DHAVE_APPLY_OUTDEG=0
CLFLAGS  += -DHAVE_APPLY_OUTDEG=0
endif



ifeq ($(strip $(CUSTOMIZE_APPLY)), $(strip $(VAR_TRUE)))
CXXFLAGS += -DCUSTOMIZE_APPLY=1
CLFLAGS  += -DCUSTOMIZE_APPLY=1
else
CXXFLAGS += -DCUSTOMIZE_APPLY=0
CLFLAGS  += -DCUSTOMIZE_APPLY=0
endif


#############################################################################
#                                                                           #
#                     Specific Configuration                                #
#                                                                           #
#############################################################################

## for hardware
CLFLAGS  += -DQUEUE_SIZE_FILTER=$(QUEUE_SIZE_FILTER)
CLFLAGS  += -DQUEUE_SIZE_MEMORY=$(QUEUE_SIZE_MEMORY)
CLFLAGS  += -DLOG_SCATTER_CACHE_BURST_SIZE=$(LOG_SCATTER_CACHE_BURST_SIZE)


CLFLAGS  += -DAPPLY_REF_ARRAY_SIZE=$(APPLY_REF_ARRAY_SIZE)
CXXFLAGS  += -DAPPLY_REF_ARRAY_SIZE=$(APPLY_REF_ARRAY_SIZE)


ifdef TARGET_PARTITION_SIZE
CLFLAGS  += -DTARGET_PARTITION_SIZE=$(TARGET_PARTITION_SIZE)
CXXFLAGS  += -DTARGET_PARTITION_SIZE=$(TARGET_PARTITION_SIZE)

endif
