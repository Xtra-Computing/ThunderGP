#include <cmath>
#include <cstdio>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include "common.h"
#include "device_common.h"

#ifndef DEVICE_HEADER
#error  "board name miss!"
#endif

#ifndef TARGET_BANDWIDTH
#error "TARGET_BANDWIDTH do not defined!"
#endif

#include STRINGIFY_MACRO(EXPAND(DEVICE_HEADER))

#define SCATTER_URAM_COST               (40)
#define URAM_CAPACITY                   (8192)
#define MAX_SUPPROTED_KERNEL            (128)
#define URAM_UPBOUND                    (0.9f)

typedef struct
{
    int uram;
    int luts;
    int id;
    int mem_chn;
} slr_info_t;

using namespace std;

int partition_size = 0;
int kernel_num = 0;

std::vector<int> kernel_mapping;
int apply_mapping = 0;

std::vector<int> memory_mapping;

int main(int argc, char **argv) {
    std::ofstream file;
    file.open("./tmp_para/para.mk");
    std::vector<slr_info_t> slr_ordered;
    DEBUG_PRINTF("board: %s \n", board_name);
    int max_bw_required = 0;
    int platform_max_memory_bandwidth = platform_memory_bandwidth_per_channel * ARRAY_SIZE(mem_cu_map);
    if (TARGET_BANDWIDTH > platform_max_memory_bandwidth)
    {
        max_bw_required = platform_max_memory_bandwidth;
    }
    else
    {
        max_bw_required = TARGET_BANDWIDTH;
    }

    int channel_num = max_bw_required / platform_memory_bandwidth_per_channel + 1;

    if (channel_num > ARRAY_SIZE(mem_cu_map))
    {
        channel_num = ARRAY_SIZE(mem_cu_map);
    }
    DEBUG_PRINTF("\t targeted memory bandwidth: %d GB/s\n", max_bw_required);
    DEBUG_PRINTF("\t utilzated memory channel: %d/%ld\n",
                 channel_num, ARRAY_SIZE(mem_cu_map));

    /* partition size */
    int max_avaliable_uram = 0;
    int max_gather_uram = 0;
    for (int i = 0; i < ARRAY_SIZE(slrs); i++)
    {
        if (max_avaliable_uram < slrs[i].uram)
        {
            max_avaliable_uram = slrs[i].uram;
        }
    }
    max_gather_uram = max_avaliable_uram - SCATTER_URAM_COST;
    int max_partition_size = 1 << ((int)log2(max_gather_uram * URAM_CAPACITY));
    DEBUG_PRINTF("\t max_gather_uram: %d\n", max_gather_uram);
    DEBUG_PRINTF("\t max_partition_size: %d\n", max_partition_size);


    int uram_cost = 0;
    while (true)
    {
        int est_urams = max_partition_size / URAM_CAPACITY;
        kernel_num = 0;
        for (int i = 0; i < ARRAY_SIZE(slrs); i++)
        {
            // avoid run out all urams,
            kernel_num += ( slrs[i].uram * URAM_UPBOUND ) / (est_urams + SCATTER_URAM_COST);
        }
        DEBUG_PRINTF("\t\t partition_size: %d, group_num: %d \n",
                     max_partition_size, kernel_num);
        if (kernel_num <  channel_num)
        {
            max_partition_size = max_partition_size / 2 ;
        }
        else
        {
            partition_size = max_partition_size;

            break;
        }
    }
    // up-bound number of kernels,
    // as one gather scatter kernel can fully utilzated one memory channel
    if (kernel_num > channel_num)
    {
        kernel_num = channel_num;
    }
    uram_cost = partition_size / URAM_CAPACITY + SCATTER_URAM_COST;


    DEBUG_PRINTF("\t final partition size: %d kernel num: %d\n",
                 partition_size, kernel_num);

    std::vector<int> sort;
    for (int i = 0; i < ARRAY_SIZE(slrs); i++)
    {
        sort.push_back(slrs[i].slr_id);
    }

    for (int k = 0; k < sort.size(); k++)
    {

        for (int j = 0; j < sort.size() - k - 1; j++)
        {
            int current = 0;
            int next = 0;
            for (int tmp = 0; tmp < sort.size(); tmp ++)
            {
                if (slrs[sort[tmp]].slr_id == j)
                {
                    current = tmp;
                }
                if (slrs[sort[tmp]].slr_id == j + 1)
                {
                    next = tmp;
                }
            }
            if (slrs[current].uram < slrs[next].uram)
            {
                int id = sort[j];
                sort[j] = sort[j + 1];
                sort[j + 1]  = id;
            }
        }
    }

    for (int i = 0; i < ARRAY_SIZE(slrs); i++)
    {
        slr_info_t resource;
        resource.id      = sort[i];
        resource.uram    = slrs[sort[i]].uram * URAM_UPBOUND;
        resource.luts    = slrs[sort[i]].luts;
        resource.mem_chn = slrs[sort[i]].mem_chns;
        slr_ordered.push_back(resource);
    }


    for (int i = 0; i < kernel_num; i++)
    {
        for (int j = 0; j < slr_ordered.size(); j++)
        {
            if (slr_ordered[j].uram > uram_cost)
            {
                kernel_mapping.push_back(slr_ordered[j].id);
                slr_ordered[j].uram -= uram_cost;
                break;
            }
        }
    }
    DEBUG_PRINTF("\t kernel mapping\n");
    for (int i = 0; i < kernel_mapping.size(); i ++)
    {
        DEBUG_PRINTF("\t\t gather scatter%d --> slr%d\n", i, kernel_mapping[i]);
    }
    for (int i = 0; i < slr_ordered.size(); i++)
    {
        DEBUG_PRINTF("\t\t lefted uram %d@%d --> slr%d\n",
                   slr_ordered[i].id, i, slr_ordered[i].uram);
    }


    int min_id = 0;
    int max_id = 0;
    int lut_max_id = 0;
    for (int i = 0; i < slr_ordered.size(); i ++)
    {
        if (slr_ordered[i].uram < slr_ordered[min_id].uram)
        {
            min_id = i;
        }
        if (slr_ordered[i].uram > slr_ordered[max_id].uram)
        {
            max_id = i;
        }
        if (slr_ordered[i].luts > slr_ordered[lut_max_id].luts)
        {
            lut_max_id = i;
        }
    }

    int diff = 0;

    for (int i = 0; i < slr_ordered.size(); i ++)
    {
        diff += slr_ordered[i].uram - slr_ordered[min_id].uram;
    }

    /* if all the slrs cost the same urams, we choose the one with max clbs(luts & ffs)*/
    DEBUG_PRINTF("\t\t apply  diff is %d\n", diff);

    if (diff == 0)
    {
        apply_mapping = slr_ordered[lut_max_id].id;
    }
    else
    {
        apply_mapping = slr_ordered[max_id].id;
    }

    DEBUG_PRINTF("\t\t apply           --> slr%d\n", apply_mapping);




    if (!file.is_open()) {
        DEBUG_PRINTF("error: can not open para.mk\n");
    }

    file << "# [ThunderGP] board: " << board_name << std::endl;
    file << "# Do not modify this file!" << std::endl;
    file << "KERNEL_NUM=" << kernel_num << std::endl;
    file << "SUB_PARTITION_NUM=" << kernel_num << std::endl;
    file << "AUTOGEN_CFLAG=-DSUB_PARTITION_NUM=$(SUB_PARTITION_NUM)" << std::endl;
    file << "PARTITION_SIZE=" << partition_size << std::endl;
    file << "AUTOGEN_CFLAG +=-DPARTITION_SIZE=$(PARTITION_SIZE)" << std::endl;
    for (int i = 0; i < kernel_mapping.size(); i ++)
    {
        file << "GS" << (i + 1) << "_SLR" << "=" << kernel_mapping[i] << std::endl;
    }
    file << "APPLY_SLR" << "=" << apply_mapping << std::endl;

#define INVALID_FLAG  0xffff

    std::vector<int> mem_cu_map_mask;
    for (int i = 0; i < kernel_mapping.size(); i ++)
    {
        int invalid = INVALID_FLAG;
        memory_mapping.push_back(invalid);
        mem_cu_map_mask.push_back(invalid);

    }
    //direct find the corresponding memory channel
    for (int i = 0; i < memory_mapping.size(); i ++)
    {
        int slr_id = 0;
        for (int j = 0; j < slr_ordered.size(); j ++)
        {
            if (slr_ordered[j].id == kernel_mapping[i])
            {
                slr_id = j;
                break;
            }
        }

        if ((slr_ordered[slr_id].mem_chn != 0) && (memory_mapping[i] == INVALID_FLAG))
        {
            slr_ordered[slr_id].mem_chn--;
            for (int j = 0; j < ARRAY_SIZE(mem_cu_map); j++)
            {
                if (mem_cu_map[j] == kernel_mapping[i])
                {
                    memory_mapping[i] = j;
                    mem_cu_map_mask[j] = 1;
                    break;
                }
            }
        }
    }
    // second option is cross slr
    for (int i = 0; i < memory_mapping.size(); i ++)
    {
        if (memory_mapping[i] == INVALID_FLAG)
        {
            int found_flag = 0;
            for (int k = 0; k < slr_ordered.size(); k++)
            {
                int slr_id = k;
                if (slr_ordered[slr_id].mem_chn != 0)
                {
                    slr_ordered[slr_id].mem_chn--;
                    for (int j = 0; j < ARRAY_SIZE(mem_cu_map); j++)
                    {
                        if ((mem_cu_map[j] == slr_ordered[slr_id].id)
                                && (mem_cu_map_mask[j] == INVALID_FLAG))
                        {
                            memory_mapping[i] = j;
                            mem_cu_map_mask[j] = 1;
                            found_flag = 1;
                            break;
                        }
                    }
                }
                if (found_flag == 1)
                {
                    break;
                }
            }
            if (found_flag == 0)
            {
                DEBUG_PRINTF("\t Error, memory channel is not available for kernel %d\n", i);
            }
        }
    }

    for (int i = 0; i < memory_mapping.size(); i ++)
    {
        file << "GS" << (i + 1) << "_DDR" << "=" << memory_mapping[i] << std::endl;
    }

    for (int i = 0; i < memory_mapping.size(); i ++)
    {
        file << "INTERFACE_" << (i) << "_ID" << "=" << memory_mapping[i] << std::endl;
    }
    file.flush();
    file.close();

    file.open("./tmp_para/mapping.h");
    file << "const cumem_lut_t mapping_item[] =" << std::endl;
    file << "{" << std::endl;
    for (int i = 0; i < memory_mapping.size(); i ++)
    {
        file << "\t{" << std::endl;
        file << "\t\t.cu_id  = " << i << "," << std::endl;
        file << "\t\t.mem_id = XCL_MEM_DDR_BANK" << memory_mapping[i] << "," << std::endl;
        file << "\t\t.he_attr_id = ATTR_PL_DDR" << memory_mapping[i] << "," << std::endl;
        file << "\t\t.interface_id = " << memory_mapping[i] << "," << std::endl;
        file << "\t}," << std::endl;
    }
    file << "};" << std::endl;

    file.flush();
    file.close();


    return 0;
}
