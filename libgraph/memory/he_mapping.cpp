#include "common.h"
#include "he_mem.h"

typedef struct
{
    int cu_id;
    int mem_id;
    int he_attr_id;
    int interface_id;
} cumem_lut_t;


const cumem_lut_t mapping_item[] =
{
    {
        .cu_id  = 0,
        .mem_id = XCL_MEM_DDR_BANK3,
        .he_attr_id = ATTR_PL_DDR3,
        .interface_id = 2,
    },
    {
        .cu_id  = 1,
        .mem_id = XCL_MEM_DDR_BANK2,
        .he_attr_id = ATTR_PL_DDR2,
        .interface_id = 1,

    },
    {
        .cu_id  = 2,
        .mem_id = XCL_MEM_DDR_BANK1,
        .he_attr_id = ATTR_PL_DDR1,
        .interface_id = 0,
    },
    {
        .cu_id  = 3,
        .mem_id = XCL_MEM_DDR_BANK0,
        .he_attr_id = ATTR_PL_DDR0,
        .interface_id = 3,
    },
};


int he_get_mem_attr(int attr_id)
{
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        if (mapping_item[i].he_attr_id == attr_id)
        {
            return mapping_item[i].mem_id;
        }
    }
    return (mapping_item[SUB_PARTITION_NUM - 1].mem_id);
}


int he_get_interface_id(int cu_id)
{
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        if (mapping_item[i].cu_id == cu_id)
        {
            return mapping_item[i].interface_id;
        }
    }
    return (mapping_item[SUB_PARTITION_NUM - 1].interface_id);
}
