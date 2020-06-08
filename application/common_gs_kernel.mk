$(XCLBIN)/readEdgesCU1.$(TARGET).$(DSA).xo: $(GS_KERNEL_PATH)/graph_fpga_cu1.cpp
	mkdir -p $(XCLBIN)
	$(XOCC) $(CLFLAGS) -c -k readEdgesCU1 -I'$(<D)' -o'$@' '$<'
BINARY_CONTAINER_OBJS += $(XCLBIN)/readEdgesCU1.$(TARGET).$(DSA).xo
BINARY_LINK_OBJS    += --nk  readEdgesCU1:1
BINARY_LINK_OBJS    += --sp  readEdgesCU1_1.edgeScoreMap:DDR[3]
BINARY_LINK_OBJS    += --sp  readEdgesCU1_1.edges:DDR[3]
BINARY_LINK_OBJS    += --sp  readEdgesCU1_1.vertexScore:DDR[3]
BINARY_LINK_OBJS    += --sp  readEdgesCU1_1.tmpVertexProp:DDR[3]
ifeq ($(strip $(HAVE_EDGE_PROP)), $(strip $(VAR_TRUE)))
BINARY_LINK_OBJS    += --sp  readEdgesCU1_1.edgeProp:DDR[3]
endif
BINARY_LINK_OBJS    += --slr readEdgesCU1_1:SLR2


ifeq ($(strip $(HAVE_FULL_SLR)), $(strip $(VAR_TRUE))) 

$(XCLBIN)/readEdgesCU2.$(TARGET).$(DSA).xo: $(GS_KERNEL_PATH)/graph_fpga_cu2.cpp
	mkdir -p $(XCLBIN)
	$(XOCC) $(CLFLAGS) -c -k readEdgesCU2 -I'$(<D)' -o'$@' '$<'
BINARY_CONTAINER_OBJS += $(XCLBIN)/readEdgesCU2.$(TARGET).$(DSA).xo
BINARY_LINK_OBJS    += --nk  readEdgesCU2:1
BINARY_LINK_OBJS    += --sp  readEdgesCU2_1.edgeScoreMap:DDR[2]
BINARY_LINK_OBJS    += --sp  readEdgesCU2_1.edges:DDR[2]
BINARY_LINK_OBJS    += --sp  readEdgesCU2_1.vertexScore:DDR[2]
BINARY_LINK_OBJS    += --sp  readEdgesCU2_1.tmpVertexProp:DDR[2]
ifeq ($(strip $(HAVE_EDGE_PROP)), $(strip $(VAR_TRUE)))
BINARY_LINK_OBJS    += --sp  readEdgesCU2_1.edgeProp:DDR[2]
endif
BINARY_LINK_OBJS    += --slr readEdgesCU2_1:SLR2



$(XCLBIN)/readEdgesCU3.$(TARGET).$(DSA).xo: $(GS_KERNEL_PATH)/graph_fpga_cu3.cpp
	mkdir -p $(XCLBIN)
	$(XOCC) $(CLFLAGS) -c -k readEdgesCU3 -I'$(<D)' -o'$@' '$<'
BINARY_CONTAINER_OBJS += $(XCLBIN)/readEdgesCU3.$(TARGET).$(DSA).xo
BINARY_LINK_OBJS    += --nk  readEdgesCU3:1
BINARY_LINK_OBJS    += --sp  readEdgesCU3_1.edgeScoreMap:DDR[1]
BINARY_LINK_OBJS    += --sp  readEdgesCU3_1.edges:DDR[1]
BINARY_LINK_OBJS    += --sp  readEdgesCU3_1.vertexScore:DDR[1]
BINARY_LINK_OBJS    += --sp  readEdgesCU3_1.tmpVertexProp:DDR[1]
ifeq ($(strip $(HAVE_EDGE_PROP)), $(strip $(VAR_TRUE)))
BINARY_LINK_OBJS    += --sp  readEdgesCU3_1.edgeProp:DDR[1]
endif
BINARY_LINK_OBJS    += --slr readEdgesCU3_1:SLR0



$(XCLBIN)/readEdgesCU4.$(TARGET).$(DSA).xo: $(GS_KERNEL_PATH)/graph_fpga_cu4.cpp
	mkdir -p $(XCLBIN)
	$(XOCC) $(CLFLAGS) -c -k readEdgesCU4 -I'$(<D)' -o'$@' '$<'
BINARY_CONTAINER_OBJS += $(XCLBIN)/readEdgesCU4.$(TARGET).$(DSA).xo
BINARY_LINK_OBJS    += --nk  readEdgesCU4:1
BINARY_LINK_OBJS    += --sp  readEdgesCU4_1.edgeScoreMap:DDR[0]
BINARY_LINK_OBJS    += --sp  readEdgesCU4_1.edges:DDR[0]
BINARY_LINK_OBJS    += --sp  readEdgesCU4_1.vertexScore:DDR[0]
BINARY_LINK_OBJS    += --sp  readEdgesCU4_1.tmpVertexProp:DDR[0]
ifeq ($(strip $(HAVE_EDGE_PROP)), $(strip $(VAR_TRUE)))
BINARY_LINK_OBJS    += --sp  readEdgesCU4_1.edgeProp:DDR[0]
endif
BINARY_LINK_OBJS    += --slr readEdgesCU4_1:SLR0

endif