#include "device_common.h"

const char * board_name = "vcu1525";

const slr_resource_info_t slrs[] =
{
    {
        .slr_id  = 0,
        .luts    = 354830,
        .ffs     = 723371,
        .dsp     = 2265,
        .bram    = 638,
        .uram    = 320,
        .mem_chns= 1,
    },
    {
        .slr_id  = 1,
        .luts    = 159088,
        .ffs     = 329162,
        .dsp     = 1317,
        .bram    = 326,
        .uram    = 160,
        .mem_chns= 2,
    },
    {
        .slr_id  = 2,
        .luts    = 354934,
        .ffs     = 723328,
        .dsp     = 2265,
        .bram    = 638,
        .uram    = 320,
        .mem_chns= 1,
    },
};

const int mem_cu_map[] =
{
    0, 1, 1, 2
};

const int platform_memory_bandwidth_per_channel = 19;
