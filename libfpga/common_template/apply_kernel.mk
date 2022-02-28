ifeq ($(strip $(HAVE_APPLY)), $(strip $(VAR_TRUE)))
$(XCLBIN)/vertexApply.$(TARGET).$(DSA).xo: $(APPLY_KERNEL_PATH)/apply_top.cpp
	mkdir -p $(XCLBIN)
	$(XOCC) $(CLFLAGS) -c -k vertexApply -I'$(<D)' -o'$@' '$<'
BINARY_CONTAINER_OBJS += $(XCLBIN)/vertexApply.$(TARGET).$(DSA).xo

BINARY_LINK_OBJS    += --connectivity.nk  vertexApply:1
BINARY_LINK_OBJS    += --connectivity.sp  vertexApply_1.vertexProp:HBM[1]

BINARY_LINK_OBJS    += --connectivity.sp  vertexApply_1.newVertexProp%THUNDERGP_VERTEXPROP_ID%:HBM[%THUNDERGP_APPLY_PORT_ID%]

BINARY_LINK_OBJS    += --connectivity.sp  vertexApply_1.tmpVertexProp%THUNDERGP_VERTEXPROP_ID%:HBM[%THUNDERGP_APPLY_PORT_ID%]

ifeq ($(strip $(HAVE_APPLY_OUTDEG)), $(strip $(VAR_TRUE)))
BINARY_LINK_OBJS    += --connectivity.sp  vertexApply_1.outDegree:HBM[0]
endif

BINARY_LINK_OBJS    += --connectivity.sp  vertexApply_1.outReg:HBM[0]
BINARY_LINK_OBJS    += --connectivity.slr vertexApply_1:SLR0

include $(APPCONFIG)/apply_kernel.mk
endif
