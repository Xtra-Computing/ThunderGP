TARGETS := hw
# emu or acc:
#   hw
#   hw_emu
# export XCL_EMULATION_MODE hw_emu

APP := pr
# pass in by app=

TARGET_BANDWIDTH := 77
# target memory bandwidth in GB/s
# max: 77GB/s
# this value can be overridden by $(app)/build.mk

DEVICES := xilinx_u50_gen3x16_xdma_201920_3

# device list:
#   xilinx_vcu1525_xdma_201830_1
#   xilinx_u200_xdma_201830_2
#   xilinx_u250_xdma_201830_2
#	xilinx_vcu1525_xdma_201830_1
