############# gs 1 #############
$(XCLBIN)/readEdgesCU1.$(TARGET).$(DSA).xo: $(GS_KERNEL_PATH)/scatter_gather_top_1.cpp
	mkdir -p $(XCLBIN)
	$(XOCC) $(CLFLAGS) -c -k readEdgesCU1 -I'$(<D)' -o'$@' '$<'
BINARY_CONTAINER_OBJS += $(XCLBIN)/readEdgesCU1.$(TARGET).$(DSA).xo
BINARY_LINK_OBJS    += --nk  readEdgesCU1:1
BINARY_LINK_OBJS    += --sp  readEdgesCU1_1.edgesHeadArray:HBM[0]
BINARY_LINK_OBJS    += --sp  readEdgesCU1_1.edgesTailArray:HBM[1]
BINARY_LINK_OBJS    += --sp  readEdgesCU1_1.vertexPushinProp:HBM[2]
BINARY_LINK_OBJS    += --sp  readEdgesCU1_1.tmpVertexProp:HBM[3]
ifeq ($(strip $(HAVE_EDGE_PROP)), $(strip $(VAR_TRUE)))
BINARY_LINK_OBJS    += --sp  readEdgesCU1_1.edgeProp:HBM[4]
endif
BINARY_LINK_OBJS    += --slr readEdgesCU1_1:SLR$(GS1_SLR)
