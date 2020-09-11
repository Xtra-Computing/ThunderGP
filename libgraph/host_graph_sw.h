#ifndef __HOST_GRAPH_SW_H__
#define __HOST_GRAPH_SW_H__

#include "para_check.h"
#include <stdint.h>

#include "common.h"
#include "global_config.h"

#include "graph.h"

#include "he_mem.h"
#include "he_mem_id.h"



#define MAX_PARTITIONS_NUM      (128)

#define DEFAULT_KERNEL_ID       (0)



#define checkStatus(str) {                          \
    if (status != 0 || status != CL_SUCCESS) {      \
        printf("Error code: %d\n", status);         \
        printf("Error: %s\n", str);                 \
        acceleratorDeinit();                        \
        exit(-1);                                   \
    }                                               \
}

typedef struct
{
    int vertexNum;
    int compressedVertexNum;
    int edgeNum;
    int blkNum;
} graphInfo;

typedef struct
{
    double fpgaExeTime;
    double effic;
    double compress;
    double degree;
} profileLog;

typedef struct
{
    unsigned int  listStart;
    unsigned int  listEnd;
    unsigned int  dstVertexStart;
    unsigned int  dstVertexEnd;
    unsigned int  srcVertexStart;
    unsigned int  srcVertexEnd;
    float         scatterCacheRatio;
    float         compressRatio;
    he_mem_t      edgeTail;
    he_mem_t      edgeHead;
    he_mem_t      edgeProp;
    he_mem_t      tmpProp;
    unsigned int  mapedTotalIndex;
    unsigned int  cuIndex;
    profileLog log;
} subPartitionDescriptor;

typedef struct
{
    subPartitionDescriptor  *sub[SUB_PARTITION_NUM];
    int                     finalOrder[SUB_PARTITION_NUM];
    unsigned int            totalEdge;
    unsigned int            subPartitionSize;
    cl_event                syncEvent[SUB_PARTITION_NUM];
    cl_event                applyEvent;
    double                  applyExeTime;
} partitionDescriptor;

typedef struct
{
    const char* name;
    int partition_mem_attr;
    int prop_id;
    int output_id;
    cl_kernel kernel;
    he_mem_t  prop[2];
    he_mem_t  tmpProp;
} gatherScatterDescriptor;

typedef struct
{
    const char* name;
    cl_kernel kernel;
} applyDescriptor;

typedef struct
{
    CSR* csr;

    subPartitionDescriptor subPartitions[MAX_PARTITIONS_NUM * SUB_PARTITION_NUM];

    partitionDescriptor partitions[MAX_PARTITIONS_NUM];

    gatherScatterDescriptor * gsKernel[SUB_PARTITION_NUM];

    applyDescriptor * applyKernel;

    cl_command_queue gsOps[SUB_PARTITION_NUM];

    cl_command_queue applyOps;

    cl_program program;

    cl_platform_id platform;

    cl_device_id device;

    cl_context context;

} graphAccelerator;


subPartitionDescriptor * getSubPartition(int partID);

partitionDescriptor * getPartition(int partID);



gatherScatterDescriptor * getGatherScatter(int kernelID);

applyDescriptor * getApply(void);

void kernelInit(graphAccelerator * acc);

void setGsKernel(int partId, int superStep, graphInfo *info);

void setApplyKernel(int partId, int superStep, graphInfo *info);



graphAccelerator * getAccelerator(void);

int acceleratorInit(const char * name, char *file_name);

int acceleratorDataPrepare(const std::string &gName, const std::string &mode, graphInfo *info);

int acceleratorDataPreprocess(graphInfo *info);

int acceleratorSuperStep(int superStep, graphInfo *info);


int acceleratorDeinit(void);

void* acceleratorQueryRegister(void);

prop_t* acceleratorQueryProperty(int step);

int acceleratorProfile (int superStep, int runCounter, graphInfo *info, double exeTime);

int acceleratorCModelDataPreprocess(graphInfo *info);

int acceleratorCModelSuperStep(int superStep, graphInfo *info);


unsigned int dataPrepareGetArg(graphInfo *info);

int dataPrepareProperty(graphInfo *info);

int getStartIndex(void);


inline int getCuIDbyInterface(int order)
{
    return he_get_interface_id(order);
}


double getCurrentTimestamp(void);


Graph* createGraph(const std::string &gName, const std::string &mode);


extern std::vector<he_mem_t*> allocate_he_mem;

template <typename T>
T * load_from_csv(std::string file_name, int he_id, int mem_id)
{
    T data;
    DEBUG_PRINTF("id: %d\n", he_id);
    std::vector<T> load_buffer;
    std::ifstream fhandle(file_name.c_str());
    if (!fhandle.is_open()) {
        DEBUG_PRINTF("error: can not open %s \n", file_name.c_str());
        return NULL;
    }
    int tmp_cnt = 0;
    while (fhandle.peek()  != EOF )
    {
        tmp_cnt ++;
        fhandle >> data;
        load_buffer.push_back(data);
    }
    fhandle.close();

    he_mem_t *p_mem = get_he_mem(he_id);
    // create new, if not existing
    if (p_mem == NULL)
    {
        he_mem_t *mem = (he_mem_t *)memalign( 4, sizeof(he_mem_t));
        mem->id =  he_id;
        mem->name = "load";
        mem->attr = mem_id;
        mem->unit_size = sizeof(T) * load_buffer.size();
        mem->size_attr = SIZE_USER_DEFINE;

        he_mem_init(getAccelerator()->context, mem);
        allocate_he_mem.push_back(mem);
    }

    p_mem = get_he_mem(he_id);
    if (p_mem->size <  sizeof(T) * load_buffer.size())
    {
        DEBUG_PRINTF("    warning mem %d is too small\n", he_id);
    }
    int load_size = (p_mem->size < sizeof(T) * load_buffer.size()) ?
                    (p_mem->size / sizeof(T)) : load_buffer.size();
    for (int i = 0; i < load_size; i++)
    {
        (((T*)p_mem->data)[i]) = load_buffer[i];
    }
    int id = he_id;
    DEBUG_PRINTF("size %d \n", p_mem->size);
    transfer_data_to_pl(getAccelerator()->context, getAccelerator()->device, &id, 1);
    return (T*)p_mem->data;
}



template <typename T>
int output_init(int he_id, int mem_id, int ref_he_id)
{
    he_mem_t *p_ref = get_he_mem(ref_he_id);
    if (p_ref == NULL)
    {
        return -1;
    }
    he_mem_t *p_mem = get_he_mem(he_id);
    // create new, if not existing
    if (p_mem == NULL)
    {
        he_mem_t *mem = (he_mem_t *)memalign( 4, sizeof(he_mem_t));
        mem->id =  he_id;
        mem->name = "output";
        mem->attr = mem_id;
        mem->unit_size = p_ref->size;
        mem->size_attr = SIZE_USER_DEFINE;
        he_mem_init(getAccelerator()->context, mem);
        allocate_he_mem.push_back(mem);
    }

    p_mem = get_he_mem(he_id);
    int load_size = (p_mem->size) / sizeof(T);
    for (int i = 0; i < load_size; i++)
    {
        (((T*)p_mem->data)[i]) = 0;
    }
    return 0;
}


template <typename T>
int write_back_csv(std::string file_name, int he_id)
{
    std::vector<T> load_buffer;
    std::ofstream fhandle(file_name.c_str());
    if (!fhandle.is_open()) {
        DEBUG_PRINTF("error: can not open %s \n", file_name.c_str());
        exit(EXIT_FAILURE);
    }

    transfer_data_from_pl(getAccelerator()->context, getAccelerator()->device, he_id);
    const he_mem_t *p_mem = get_he_mem(he_id);
    const  T * p_data = (T *)(p_mem->data);

    for (unsigned int i = 0; i < p_mem->size / sizeof(T); i++)
    {
        fhandle << p_data[i] << std::endl;
    }
    fhandle.flush();
    fhandle.close();

    return 0;
}


#endif /* __HOST_GRAPH_SW_H__ */
