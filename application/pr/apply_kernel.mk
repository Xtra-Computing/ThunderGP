BINARY_CONTAINER_OBJS += $(XCLBIN)/vertexApply.$(TARGET).$(DSA).xo
BINARY_LINK_OBJS    += --nk  vertexApply:1
BINARY_LINK_OBJS    += --sp  vertexApply_1.vertexProp:DDR[1]


BINARY_LINK_OBJS    += --sp  vertexApply_1.outDegree:DDR[2]

BINARY_LINK_OBJS    += --sp  vertexApply_1.newVertexProp1:DDR[1]
BINARY_LINK_OBJS    += --sp  vertexApply_1.tmpVertexProp1:DDR[1]

BINARY_LINK_OBJS    += --sp  vertexApply_1.newVertexProp2:DDR[2]
BINARY_LINK_OBJS    += --sp  vertexApply_1.tmpVertexProp2:DDR[2]

BINARY_LINK_OBJS    += --sp  vertexApply_1.newVertexProp3:DDR[3]
BINARY_LINK_OBJS    += --sp  vertexApply_1.tmpVertexProp3:DDR[3]

BINARY_LINK_OBJS    += --sp  vertexApply_1.newVertexProp4:DDR[0]
BINARY_LINK_OBJS    += --sp  vertexApply_1.tmpVertexProp4:DDR[0]

BINARY_LINK_OBJS    += --sp  vertexApply_1.error:DDR[2]
BINARY_LINK_OBJS    += --slr vertexApply_1:SLR1