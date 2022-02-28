#include "common.h"
#include "he_mem.h"

typedef struct
{
    int cu_id;
    int mem_id;
    int he_attr_id;
    int interface_id;
} cumem_lut_t;


typedef struct
{
    int interface_id;
    int mem_id;
    int he_attr_id;
} attr_lut_t;

const attr_lut_t attr_mapping[] =
{
    {
        .interface_id = 0,
        .mem_id       = 0 | XCL_MEM_TOPOLOGY,
        .he_attr_id   = ATTR_PL_DDR0,
    },
    {
        .interface_id = 1,
        .mem_id       = 1 | XCL_MEM_TOPOLOGY,
        .he_attr_id   = ATTR_PL_DDR1,
    },
    {
        .interface_id = 2,
        .mem_id       = 2 | XCL_MEM_TOPOLOGY,
        .he_attr_id   = ATTR_PL_DDR2,
    },

    {
        .interface_id = 3,
        .mem_id       = 3 | XCL_MEM_TOPOLOGY,
        .he_attr_id   = ATTR_PL_DDR3,
    },
};


// #include "mapping.h"

// int he_get_mem_attr(int attr_id)
// {
// #if HAVE_APPLY
//     int total_mapping_items = ARRAY_SIZE(attr_mapping);
// #else
//     int total_mapping_items = SUB_PARTITION_NUM;
// #endif
//     for (int i = 0; i < total_mapping_items; i++)
//     {
//         if (attr_mapping[i].he_attr_id == attr_id)
//         {
//             return attr_mapping[i].mem_id;
//         }
//     }
//     return (attr_mapping[SUB_PARTITION_NUM - 1].mem_id);
// }


// int he_get_interface_id(int cu_id)
// {
//     for (int i = 0; i < SUB_PARTITION_NUM; i++)
//     {
//         if (mapping_item[i].cu_id == cu_id)
//         {
//             return mapping_item[i].interface_id;
//         }
//     }
//     return (mapping_item[SUB_PARTITION_NUM - 1].interface_id);
// }


// int he_get_attr_by_cu(int cu_id)
// {
//     for (int i = 0; i < SUB_PARTITION_NUM; i++)
//     {
//         if (mapping_item[i].cu_id == cu_id)
//         {
//             return mapping_item[i].he_attr_id;
//         }
//     }
//     return (mapping_item[SUB_PARTITION_NUM - 1].he_attr_id);
// }
