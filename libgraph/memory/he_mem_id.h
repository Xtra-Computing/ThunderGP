#ifndef __HE_MEM_ID_H__
#define __HE_MEM_ID_H__



#define SIZE_IN_EDGE                (0)
#define SIZE_IN_VERTEX              (1)
#define SIZE_USER_DEFINE            (2)





#define MEM_ID_EDGE_TUPLES          (1)
#define MEM_ID_EDGE_SCORE_MAP       (2)
#define MEM_ID_VERTEX_SCORE         (3)
#define MEM_ID_VERTEX_PROP          (4)
#define MEM_ID_TMP_VERTEX_PROP      (5)
#define MEM_ID_TMP_VERTEX_VERIFY    (6)
#define MEM_ID_OUT_DEG              (7)
#define MEM_ID_ERROR                (8)
#define MEM_ID_RPA                  (10)
#define MEM_ID_CIA                  (11)
#define MEM_ID_EDGE_PROP            (12)
#define MEM_ID_VERTEX_PROP_VERIFY   (14)
#define MEM_ID_VERTEX_SCORE_MAPPED  (15)
#define MEM_ID_VERTEX_INDEX_MAP     (16)
#define MEM_ID_VERTEX_INDEX_REMAP   (17)
#define MEM_ID_VERTEX_INDEX_BIT_ORI (18)
#define MEM_ID_VERTEX_INDEX_BIT     (19)

#define MEM_ID_ACTIVE_VERTEX        (20)
#define MEM_ID_ACTIVE_VERTEX_NUM    (21)

#define MEM_ID_OUT_DEG_ORIGIN       (22)

#define MEM_ID_PARTITION_BASE       (100)

#define MEM_ID_PARTITION_OFFSET     (128)


#define MEM_ID_GS_BASE              (8192)
#define MEM_ID_GS_OFFSET            (128)


#endif /* __HE_MEM_ID_H__ */
