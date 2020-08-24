SHELL := /bin/bash
COMMON_REPO = ./
ABS_COMMON_REPO = $(shell readlink -f $(COMMON_REPO))
UTILS_PATH = ./utils

include $(UTILS_PATH)/help.mk
include $(UTILS_PATH)/utils.mk

TARGETS := hw
TARGET  := $(TARGETS)
DEVICES := xilinx_vcu1525_xdma_201830_1
# device list:
# xilinx_vcu1525_xdma_201830_1
# xilinx_u200_xdma_201830_2
# xilinx_u250_xdma_201830_2

DEVICE  := $(DEVICES)

.PHONY:all clean emconfig exe
all:
ifndef app
	$(error app is undefined)
else
APP = $(app)
APPCONFIG = ./application/$(APP)

include $(APPCONFIG)/config.mk
include $(APPCONFIG)/build.mk
include ./application/common.mk

CODE_GEN_PATH=./libfpga/generator
include $(CODE_GEN_PATH)/parser.mk


all: code_gen $(EXECUTABLE) $(BINARY_CONTAINERS) emconfig
exe: cleanexe $(EXECUTABLE)

include ./application/common_gs_kernel.mk
include ./application/common_apply_kernel.mk

include $(UTILS_PATH)/bitstream.mk
include $(UTILS_PATH)/clean.mk

endif
