#ifndef __MEM_CONFIG_H__
#define __MEM_CONFIG_H__



he_mem_t local_mem[] =
{
    {
        MEM_ID_PROP_FOR_DATAPREPARE,
        "data prepare",
        ATTR_HOST_ONLY,
        sizeof(prop_t),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_TEST,
        "test",
        ATTR_HOST_ONLY,
        sizeof(prop_t),
        SIZE_IN_VERTEX,
    },

    // tmp property
    {
        MEM_ID_TMP_VERTEX_PROP,
        "tmpVertexProp",
        ATTR_HOST_ONLY,
        sizeof(prop_t),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_VERTEX_PROP_VERIFY,
        "vertexProp for verification",
        ATTR_HOST_ONLY,
        sizeof(prop_t),
        SIZE_IN_VERTEX,
    },

    {
        MEM_ID_EDGE_TAIL,
        "edgesTailArray",
        ATTR_HOST_ONLY,
        sizeof(prop_t),
        SIZE_IN_EDGE,
    },
    {
        MEM_ID_EDGE_HEAD,
        "edgesHeadArray",
        ATTR_HOST_ONLY,
        sizeof(int),
        SIZE_IN_EDGE,
    },
    {
        MEM_ID_PUSHIN_PROP,
        "vertexPushinProp",
        ATTR_HOST_ONLY,
        sizeof(int),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_PUSHIN_PROP_MAPPED,
        "vertexPushinPropMapped",
        ATTR_HOST_ONLY,
        sizeof(int),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_HOST_PROP_PING,
        "hostVerificationPropPing",
        ATTR_HOST_ONLY,
        sizeof(prop_t),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_HOST_PROP_PONG,
        "hostVerificationPropPong",
        ATTR_HOST_ONLY,
        sizeof(prop_t),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_TMP_VERTEX_VERIFY,
        "tmpVertexPropVerify",
        ATTR_HOST_ONLY,
        sizeof(prop_t),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_OUT_DEG,
        "outDeg",
        ATTR_PL_DDR0,
        sizeof(int),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_OUT_DEG_ORIGIN,
        "outDeg origin",
        ATTR_HOST_ONLY,
        sizeof(int),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_RESULT_REG,
        "error",
        ATTR_PL_DDR0,
        sizeof(int) * 64,
        SIZE_USER_DEFINE,
    },
    {
        MEM_ID_RPA,
        "rpa",
        ATTR_HOST_ONLY,
        sizeof(int) * 2,
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_CIA,
        "cia",
        ATTR_HOST_ONLY,
        sizeof(int) * 2,
        SIZE_IN_EDGE,
    },
    {
        MEM_ID_EDGE_PROP,
        "edgeProp",
        ATTR_HOST_ONLY,
        sizeof(int),
        SIZE_IN_EDGE,
    },
    {
        MEM_ID_PARTITON_EDGE_PROP,
        "edgePartitionProp",
        ATTR_HOST_ONLY,
        sizeof(int),
        SIZE_IN_EDGE,
    },
    {
        MEM_ID_VERTEX_INDEX_MAP,
        "indexMap",
        ATTR_HOST_ONLY,
        sizeof(int),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_VERTEX_INDEX_REMAP,
        "indexRemap",
        ATTR_HOST_ONLY,
        sizeof(int),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_VERTEX_INDEX_BIT_ORI,
        "sourceBitmapOri",
        ATTR_HOST_ONLY,
        sizeof(int),
        SIZE_IN_EDGE,
    },
    {
        MEM_ID_VERTEX_INDEX_BIT,
        "sourceBitmap",
        ATTR_HOST_ONLY,
        sizeof(int),
        SIZE_IN_EDGE,
    },
    {
        MEM_ID_ACTIVE_VERTEX_NUM,
        "activeVertexNum",
        ATTR_HOST_ONLY,
        sizeof(prop_t),
        SIZE_IN_VERTEX,
    },
};


#endif /* __MEM_CONFIG_H__ */

