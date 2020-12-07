ifeq ($(strip $(HAVE_APPLY)), $(strip $(VAR_TRUE)))
$(XCLBIN)/vertexApply.$(TARGET).$(DSA).xo: $(APPLY_KERNEL_PATH)/apply_top_1.cpp
	mkdir -p $(XCLBIN)
	$(XOCC) $(CLFLAGS) -c -k vertexApply -I'$(<D)' -o'$@' '$<'
BINARY_CONTAINER_OBJS += $(XCLBIN)/vertexApply.$(TARGET).$(DSA).xo
BINARY_LINK_OBJS    += --nk  vertexApply:1
BINARY_LINK_OBJS    += --sp  vertexApply_1.vertexProp:HBM[2]
BINARY_LINK_OBJS    += --sp  vertexApply_1.newVertexProp0:HBM[2]
BINARY_LINK_OBJS    += --sp  vertexApply_1.tmpVertexProp0:HBM[3]

ifeq ($(strip $(HAVE_APPLY_OUTDEG)), $(strip $(VAR_TRUE)))
BINARY_LINK_OBJS    += --sp  vertexApply_1.outDegree:HBM[5]
endif
BINARY_LINK_OBJS    += --sp  vertexApply_1.outReg:HBM[5]
BINARY_LINK_OBJS    += --slr vertexApply_1:SLR$(APPLY_SLR)
include $(APPCONFIG)/apply_kernel.mk
endif
