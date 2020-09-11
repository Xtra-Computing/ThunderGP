#include "device_common.h"

const char * board_name = "u250";

const slr_resource_info_t slrs[] =
{
    {
        .slr_id  = 0,
        .luts    = 345171,
        .ffs     = 704801,
        .dsp     = 2877,
        .bram    = 500,
        .uram    = 320,
        .mem_chns= 1,
    },
    {
        .slr_id  = 1,
        .luts    = 344533,
        .ffs     = 702517,
        .dsp     = 2877,
        .bram    = 500,
        .uram    = 320,
        .mem_chns= 1,
    },
    {
        .slr_id  = 2,
        .luts    = 344878,
        .ffs     = 703253,
        .dsp     = 2877,
        .bram    = 500,
        .uram    = 320,
        .mem_chns= 1,
    },
    {
        .slr_id  = 3,
        .luts    = 345158,
        .ffs     = 703761,
        .dsp     = 2877,
        .bram    = 500,
        .uram    = 320,
        .mem_chns= 1,
    },
};

const int mem_cu_map[] =
{
    0, 1, 2, 3
};

const int platform_memory_bandwidth_per_channel = 19;
