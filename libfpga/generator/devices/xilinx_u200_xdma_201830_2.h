#include "device_common.h"

const char * board_name = "u200";

const slr_resource_info_t slrs[] =
{
    {
        .slr_id  = 0,
        .luts    = 354831,
        .ffs     = 723372,
        .dsp     = 2265,
        .bram    = 638,
        .uram    = 320,
        .mem_chns= 1,
    },
    {
        .slr_id  = 1,
        .luts    = 159854,
        .ffs     = 331711,
        .dsp     = 1317,
        .bram    = 326,
        .uram    = 160,
        .mem_chns= 2,
    },
    {
        .slr_id  = 2,
        .luts    = 354962,
        .ffs     = 723353,
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
