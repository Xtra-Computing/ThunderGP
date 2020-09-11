ifeq ($(strip $(HAVE_APPLY)), $(strip $(VAR_TRUE)))
$(XCLBIN)/vertexApply.$(TARGET).$(DSA).xo: $(APPLY_KERNEL_PATH)/apply_top_1.cpp
	mkdir -p $(XCLBIN)
	$(XOCC) $(CLFLAGS) -c -k vertexApply -I'$(<D)' -o'$@' '$<'
BINARY_CONTAINER_OBJS += $(XCLBIN)/vertexApply.$(TARGET).$(DSA).xo
BINARY_LINK_OBJS    += --nk  vertexApply:1
BINARY_LINK_OBJS    += --sp  vertexApply_1.vertexProp:DDR[$(GS1_DDR)]
#pragma THUNDERGP MAKEFILE_MEM_INSTANCE
BINARY_LINK_OBJS    += --sp  vertexApply_1.newVertexProp$(%d):DDR[$(INTERFACE_$(%d)_ID)]
#pragma THUNDERGP MAKEFILE_MEM_INSTANCE
BINARY_LINK_OBJS    += --sp  vertexApply_1.tmpVertexProp$(%d):DDR[$(INTERFACE_$(%d)_ID)]

ifeq ($(strip $(HAVE_APPLY_OUTDEG)), $(strip $(VAR_TRUE)))
BINARY_LINK_OBJS    += --sp  vertexApply_1.outDegree:DDR[2]
endif
BINARY_LINK_OBJS    += --sp  vertexApply_1.outReg:DDR[2]
BINARY_LINK_OBJS    += --slr vertexApply_1:SLR$(APPLY_SLR)
include $(APPCONFIG)/apply_kernel.mk
endif
