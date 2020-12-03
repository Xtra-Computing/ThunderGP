ifeq ($(strip $(HAVE_APPLY)), $(strip $(VAR_TRUE)))
$(XCLBIN)/vertexApply.$(TARGET).$(DSA).xo: $(APPLY_KERNEL_PATH)/apply_top_1.cpp
	mkdir -p $(XCLBIN)
	$(XOCC) $(CLFLAGS) -c -k vertexApply -I'$(<D)' -o'$@' '$<'
BINARY_CONTAINER_OBJS += $(XCLBIN)/vertexApply.$(TARGET).$(DSA).xo
BINARY_LINK_OBJS    += --nk  vertexApply:1
BINARY_LINK_OBJS    += --sp  vertexApply_1.vertexProp:HBM[$(GS1_DDR)]
BINARY_LINK_OBJS    += --sp  vertexApply_1.newVertexProp0:HBM[$(INTERFACE_0_ID)]
BINARY_LINK_OBJS    += --sp  vertexApply_1.newVertexProp1:HBM[$(INTERFACE_1_ID)]
BINARY_LINK_OBJS    += --sp  vertexApply_1.newVertexProp2:HBM[$(INTERFACE_2_ID)]
BINARY_LINK_OBJS    += --sp  vertexApply_1.newVertexProp3:HBM[$(INTERFACE_3_ID)]
BINARY_LINK_OBJS    += --sp  vertexApply_1.tmpVertexProp0:HBM[$(INTERFACE_0_ID)]
BINARY_LINK_OBJS    += --sp  vertexApply_1.tmpVertexProp1:HBM[$(INTERFACE_1_ID)]
BINARY_LINK_OBJS    += --sp  vertexApply_1.tmpVertexProp2:HBM[$(INTERFACE_2_ID)]
BINARY_LINK_OBJS    += --sp  vertexApply_1.tmpVertexProp3:HBM[$(INTERFACE_3_ID)]

ifeq ($(strip $(HAVE_APPLY_OUTDEG)), $(strip $(VAR_TRUE)))
BINARY_LINK_OBJS    += --sp  vertexApply_1.outDegree:HBM[2]
endif
BINARY_LINK_OBJS    += --sp  vertexApply_1.outReg:HBM[2]
BINARY_LINK_OBJS    += --slr vertexApply_1:SLR$(APPLY_SLR)
include $(APPCONFIG)/apply_kernel.mk
endif
