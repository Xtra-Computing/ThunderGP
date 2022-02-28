$(XCLBIN)/readEdgesCU1.$(TARGET).$(DSA).xo: $(GS_KERNEL_PATH)/scatter_gather_top.cpp
	mkdir -p $(XCLBIN)
	$(XOCC) $(CLFLAGS) -c -k readEdgesCU1 -I'$(<D)' -o'$@' '$<'

BINARY_CONTAINER_OBJS += $(XCLBIN)/readEdgesCU1.$(TARGET).$(DSA).xo

BINARY_LINK_OBJS    += --connectivity.nk  readEdgesCU1:%THUNDERGP_KERNEL_NUM%

BINARY_LINK_OBJS    += --connectivity.sp  readEdgesCU1_%THUNDERGP_KERNEL_NAME%.edgesHeadArray:HBM[%THUNDERGP_GS_KERNEL_EDGEARRAY_ID%]
BINARY_LINK_OBJS    += --connectivity.sp  readEdgesCU1_%THUNDERGP_KERNEL_NAME%.vertexPushinProp:HBM[%THUNDERGP_GS_KERNEL_VERTEXPROP_ID%]
BINARY_LINK_OBJS    += --connectivity.sp  readEdgesCU1_%THUNDERGP_KERNEL_NAME%.tmpVertexProp:HBM[%THUNDERGP_GS_KERNEL_VERTEXPROP_ID%]
ifeq ($(strip $(HAVE_EDGE_PROP)), $(strip $(VAR_TRUE)))
BINARY_LINK_OBJS    += --connectivity.sp  readEdgesCU1_%THUNDERGP_KERNEL_NAME%.edgeProp:HBM[%THUNDERGP_GS_KERNEL_EDGEPROP_ID%]
endif
BINARY_LINK_OBJS    += --connectivity.slr readEdgesCU1_%THUNDERGP_KERNEL_NAME%:SLR%THUNDERGP_GS_KERNEL_SLR%
