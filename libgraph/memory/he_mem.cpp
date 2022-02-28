#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "common.h"
#include "he_mem.h"
#include "he_mem_id.h"

#include "he_mem_attr.h"

//#define HE_DEBUG(fmt,...)   printf(fmt,##__VA_ARGS__); fflush(stdout);
#define HE_DEBUG(fmt,...) ;

int he_set_dirty(int id)
{
    he_mem_t * p_mem = get_he_mem(id);
    if (p_mem)
    {
        HE_DEBUG("[HE_DEBUG] 0x%08x set dirty\n", id);
        p_mem->dirty_flags = FLAG_SET;
        return 0;
    }
    return -1;
}


int he_set_clean(int id)
{
    he_mem_t * p_mem = get_he_mem(id);
    if (p_mem)
    {
        HE_DEBUG("[HE_DEBUG] 0x%08x set clean\n", id);
        p_mem->dirty_flags = FLAG_RESET;
        return 0;
    }
    return -1;
}

int he_get_dirty_flag(int id)
{
    he_mem_t * p_mem = get_he_mem(id);
    if (p_mem)
    {
        return p_mem->dirty_flags;
    }
    return -1;
}


static std::vector<he_mem_lookup_t> mem_list;

int register_size_attribute(unsigned int attr_id, int value)
{
    for (unsigned int i = 0; i < ARRAY_SIZE(local_size_ctrl); i++)
    {
        if (attr_id == local_size_ctrl[i].size_attr)
        {
            local_size_ctrl[i].scale = value;
            return 0;
        }
    }
    return -1;
}

unsigned int get_size_attribute(unsigned int attr_id)
{
    int value = 1;
    for (unsigned int i = 0; i < ARRAY_SIZE(local_size_ctrl); i++)
    {
        if (attr_id == local_size_ctrl[i].size_attr)
        {
            value = local_size_ctrl[i].scale;
            break;
        }
    }
    return value;
}



int he_mem_init(cl_context &dev_context, he_mem_t * item)
{
    cl_int status;
    if (item == NULL)
    {
        DEBUG_PRINTF("%s", "[INIT] error input of he_mem_t\n");
        return -1;
    }

    item->size = item->unit_size * get_size_attribute(item->size_attr);
    item->size = SIZE_ALIGNMENT(item->size, 64 * 1024);

    item->data = clSVMAlloc(dev_context, 0, item->size, 64 * 1024);

    memset(item->data, 0 , item->size);

    HE_DEBUG("[INIT] alignment allocate for %s in %p with size %d\n",
                 item->name,
                 item->data,
                 item->size)
    if (item->data == NULL)
    {
        DEBUG_PRINTF("[INIT] alignment allocate for %s error !\n", item->name);
        return -2;
    }
    he_mem_lookup_t lut_item;
    lut_item.id = item->id;
    lut_item.p_mem = item;
    mem_list.push_back(lut_item);
    /* TODO: check duplicate id */

    if (item->attr == ATTR_HOST_ONLY) {
        return 0;
    }
    else if (item->attr >= ATTR_ERROR) {
        DEBUG_PRINTF("[INIT] error attr %s %d\n", item->name, item->attr);
        return -3;
    }
    else if (item->attr == ATTR_PL_DEFAULT) {
        item->device = clCreateBuffer(dev_context, CL_MEM_READ_WRITE, item->size, 0, &status);
        if (status != CL_SUCCESS) {
            DEBUG_PRINTF("[INIT] cl mem error %s %d in %d\n", item->name, item->attr, status);
            exit(1);
        }
    }
    else {
#ifdef SW_DEBUG
        item->device = clCreateBuffer(dev_context, CL_MEM_READ_WRITE, item->size, 0, &status);
        if (status != CL_SUCCESS) {
            DEBUG_PRINTF("[INIT] cl mem error %s %d in %d\n", item->name, item->attr, status);
            exit(1);
        }
#else
        item->ext_attr.obj =  item->data;
        item->ext_attr.param = 0;

        // hardcode the bank for the outdeg variable... 
        if((strcmp (item->name, "outDeg") == 0) || (strcmp (item->name, "error") == 0)) item->ext_attr.flags = (0 | XCL_MEM_TOPOLOGY);

        std::cout << item->name << " is banked to " << std::hex << item->ext_attr.flags << std::endl; 

        item->device = clCreateBuffer(dev_context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR |  CL_MEM_EXT_PTR_XILINX,
                                      item->size  , &item->ext_attr, &status);
        
        item->dirty_flags = FLAG_RESET;
        if (status != CL_SUCCESS) {
            DEBUG_PRINTF("[INIT] cl mem error %s %d in %d\n", item->name, item->attr, status);
            exit(1);
        }
#endif
    }

    return 0;
}

he_mem_t* get_he_mem(unsigned int id)
{
    for (auto lut_item : mem_list)
    {
        if (id == lut_item.id)
        {
            return lut_item.p_mem;
        }
    }
    HE_DEBUG("[ERROR] get he mem error %d \n", id);
    return NULL;
}

void* get_host_mem_pointer(int id)
{
    he_mem_t * p_mem = get_he_mem(id);
    if (p_mem)
    {
        return p_mem->data;
    }
    return NULL;
}

void clear_host_mem(int id)
{
    he_mem_t * p_mem = get_he_mem(id);
    if (p_mem)
    {
        memset(p_mem->data, 0 , p_mem->size);
        return;
    }
    DEBUG_PRINTF("%s %d do not found\n", __FUNCTION__, id);
    return;
}
cl_mem* get_cl_mem_pointer(int id)
{
    he_mem_t * p_mem = get_he_mem(id);
    if (p_mem)
    {
        if (p_mem->attr == ATTR_HOST_ONLY)
        {
            DEBUG_PRINTF("[ERROR] cl mem %d %s is host only\n", id, p_mem->name );
            return NULL;
        }
        return &(p_mem->device);
    }
    DEBUG_PRINTF("cl mem %d do not found \n", id );
    return NULL;
}

int transfer_data_to_pl(cl_context &dev_context, cl_device_id device_id, int* id_array, int size)
{
    cl_int status;
    cl_command_queue ops;
    ops = clCreateCommandQueue(dev_context, device_id, CL_QUEUE_PROFILING_ENABLE, &status);
    if (status != CL_SUCCESS) {
        DEBUG_PRINTF("%s failed to create queue %d \n", __FUNCTION__, status);
        exit(1);
    }
    for (int i = 0; i < size; i++)
    {
        int mem_id = id_array[i];
        he_mem_t *p_mem = get_he_mem(mem_id);
        if (p_mem == NULL)
        {
            DEBUG_PRINTF("%s error memid %d\n", __FUNCTION__, mem_id);
            return -1;
        }
        if (p_mem->attr == ATTR_HOST_ONLY)
        {
            continue;
        }
        HE_DEBUG("[HEME] %s enqueue size %d @ %p\n", p_mem->name, p_mem->size, p_mem->data );
        status = clEnqueueMigrateMemObjects(ops, 1, &p_mem->device, 0, 0, NULL, NULL);
        if (status != CL_SUCCESS) {
            DEBUG_PRINTF("%s enqueue failed %d\n", p_mem->name, status);
            exit(1);
        }
        HE_DEBUG("[HEME] %s enqueue success\n", p_mem->name);
    }
    clFinish(ops);
    return 0;
}
int transfer_data_from_pl(cl_context &dev_context, cl_device_id device_id, int mem_id)
{
    cl_int status;
    cl_command_queue ops;
    ops = clCreateCommandQueue(dev_context, device_id, CL_QUEUE_PROFILING_ENABLE, &status);
    he_mem_t *p_mem = get_he_mem(mem_id);
    if (p_mem == NULL)
    {
        //DEBUG_PRINTF("%s error memid %d\n", __FUNCTION__, mem_id);
        return -1;
    }
    if (p_mem->dirty_flags == FLAG_RESET)
    {
        HE_DEBUG("0x%08x is clean\n", mem_id);
    }
    else
    {
        HE_DEBUG("0x%08x is dirty\n", mem_id);
        status = clEnqueueMigrateMemObjects(ops, 1, &p_mem->device, CL_MIGRATE_MEM_OBJECT_HOST, 0, NULL, NULL);
        clFinish(ops);
        p_mem->dirty_flags = FLAG_RESET;
    }
    //status = clEnqueueReadBuffer(ops, p_mem->device, CL_TRUE, 0, p_mem->size , p_mem->data, 0, NULL, NULL);

    return 0;
}

std::vector<he_mem_t *> allocate_he_mem;



int he_mem_from_bin(std::string file_name, int he_id, int mem_id)
{
    return 0;
}

