#ifndef __MEM_CONFIG_H__
#define __MEM_CONFIG_H__



he_mem_t local_mem[] =
{
    {
        MEM_ID_VERTEX_PROP,
        "vertexProp",
        ATTR_PL_DDR1,
        sizeof(PROP_TYPE),
        SIZE_IN_VERTEX,
    },
    // tmp property
    {
        MEM_ID_TMP_VERTEX_PROP,
        "tmpVertexProp",
        ATTR_HOST_ONLY,
        sizeof(PROP_TYPE),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_VERTEX_PROP_VERIFY,
        "vertexProp for verification",
        ATTR_HOST_ONLY,
        sizeof(PROP_TYPE),
        SIZE_IN_VERTEX,
    },
    
    {
        MEM_ID_EDGE_TUPLES,
        "edgesTuples",
        ATTR_HOST_ONLY,
        sizeof(PROP_TYPE),
        SIZE_IN_EDGE,
    },
    {
        MEM_ID_EDGE_SCORE_MAP,
        "edgeScoreMap",
        ATTR_HOST_ONLY,
        sizeof(int),
        SIZE_IN_EDGE,
    },
    {        
        MEM_ID_VERTEX_SCORE,
        "vertexScore",
        ATTR_HOST_ONLY,
        sizeof(int),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_VERTEX_SCORE_CACHED,
        "vertexScoreCached",
        ATTR_HOST_ONLY,
        sizeof(int),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_TMP_VERTEX_VERIFY,
        "tmpVertexPropVerify",
        ATTR_HOST_ONLY,
        sizeof(PROP_TYPE),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_TMP_VERTEX_SW,
        "tmpVertexPropSw",
        ATTR_HOST_ONLY,
        sizeof(PROP_TYPE),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_OUT_DEG,
        "outDeg",
        ATTR_PL_DDR2,
        sizeof(int),
        SIZE_IN_VERTEX,
    },
    {
        MEM_ID_ERROR,
        "error",
        ATTR_PL_DDR2,
        sizeof(int) * 4,
        SIZE_IN_VERTEX,
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
};


#endif /* __MEM_CONFIG_H__ */

