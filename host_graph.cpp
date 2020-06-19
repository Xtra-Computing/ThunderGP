
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#include <fstream>
#include <vector>

#include "unistd.h"

#include "host_graph_sw.h"

#include "host_graph_scheduler.h"

using namespace std;


int vertexNum;
int edgeNum;
int blkNum;
CSR* csr;


static cl_int status;
static cl_program program;
static cl_platform_id platform;
static cl_device_id device;
static cl_context context;


void freeResources(void) {

    /* TODO free other resource */

    if (context)            clReleaseContext(context);
}

extern gatherScatterDescriptor localGsKernel[SUB_PARTITION_NUM];
extern subPartitionDescriptor subPartitions[MAX_PARTITIONS_NUM];


void hardware_init(const char * name, char *file_name) {
    cl_uint numPlatforms;
    cl_uint numDevices;
    status = clGetPlatformIDs(1, &platform, &numPlatforms);
    checkStatus("Failed clGetPlatformIDs.");
    printf("Found %d platforms!\n", numPlatforms);

    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, &numDevices);
    checkStatus("Failed clGetDeviceIDs.");
    printf("Found %d devices!\n", numDevices);

    context = clCreateContext(0, 1, &device, NULL, NULL, &status);
    checkStatus("Failed clCreateContext.");

    xcl_world world = xcl_world_single();
    program = xcl_import_binary(world, name, file_name);

    std::cout << "set kernel env." << std::endl;
}


static cl_kernel kernel_apply;


void kernel_init(cl_program &program)
{
    static cl_int status;
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
#if HAVE_GS
        DEBUG_PRINTF("kernel init \n");
        localGsKernel[i].kernel = clCreateKernel(program, localGsKernel[i].name, &status);
        checkStatus("Failed clCreateKernel gs.")
#endif
    }
#if HAVE_APPLY
    kernel_apply = clCreateKernel(program, "vertexApply", &status);
    checkStatus("Failed clCreateKernel vertexApply.");
#endif
}




void prepare_fpga_ddr(void)
{
    DEBUG_PRINTF("%s", "transfer base mem start\n");
    double  begin = getCurrentTimestamp();


    int base_mem_id[]  = {
        MEM_ID_VERTEX_SCORE_CACHED,
        MEM_ID_OUT_DEG,
        MEM_ID_ERROR
    };
    DEBUG_PRINTF("%s", "transfer base mem\n");
    transfer_data_to_pl(context, device, base_mem_id, ARRAY_SIZE(base_mem_id));
    DEBUG_PRINTF("%s", "transfer subPartitions mem\n");
    for (int i = 0; i < blkNum; i ++) {
        int partition_mem_id[3];
        partition_mem_id[0] = subPartitions[i].edge.id;
        partition_mem_id[1] = subPartitions[i].edgeMap.id;
        partition_mem_id[2] = subPartitions[i].edgeProp.id;
        transfer_data_to_pl(context, device, partition_mem_id, ARRAY_SIZE(partition_mem_id));
    }
    DEBUG_PRINTF("%s", "transfer cu mem\n");
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        int cu_mem_id[2];
        cu_mem_id[0] = localGsKernel[i].prop.id;
        cu_mem_id[1] = localGsKernel[i].tmpProp.id;
        transfer_data_to_pl(context, device, cu_mem_id, ARRAY_SIZE(cu_mem_id));
    }

    double end =  getCurrentTimestamp();
    DEBUG_PRINTF("data transfer %lf \n", (end - begin) * 1000);
}

#define TEST_SCALE          (1)

#define GLOBAL_BLK_SIZE     (blkNum / TEST_SCALE)

static cl_command_queue gs_ops[SUB_PARTITION_NUM], apply_ops;
static cl_event  syncEvent[MAX_PARTITIONS_NUM * SUB_PARTITION_NUM];
static cl_event  applyEvent[MAX_PARTITIONS_NUM];

double launchFPGA(void)
{


    //CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        gs_ops[i] = clCreateCommandQueue(context, device,   CL_QUEUE_PROFILING_ENABLE, &status);
        checkStatus("Failed clCreateCommandQueue of gs_ops.");
    }
    apply_ops = clCreateCommandQueue(context, device,  CL_QUEUE_PROFILING_ENABLE, &status);
    checkStatus("Failed clCreateCommandQueue of apply_ops.");
    /* the first repeat:  data verfication */
    /* the second repeat: performance      */
    double runtime_total = 0;


    for (int repeat = 0 ; repeat < 3 ; repeat ++)
    {
        double fpga_run_start, fpga_run_end;

        /************************************************************************************/

        fpga_run_start = getCurrentTimestamp();
        for (int i = 0; i < GLOBAL_BLK_SIZE; i ++)
        {
            setGsKernel(getArrangedPartitionID(i));
            if (i > 0)
            {
#if HAVE_APPLY
                setApplyKernel(kernel_apply, getArrangedPartitionID(i - 1), vertexNum);
                clEnqueueTask(apply_ops, kernel_apply, 4,
                              &syncEvent[(i - 1) * SUB_PARTITION_NUM],
                              &applyEvent[i - 1]);
#endif
            }
            for (int j = 0; j < SUB_PARTITION_NUM; j ++)
            {
                localGsKernel[j].event[i] = &syncEvent[i * SUB_PARTITION_NUM + j];
                clEnqueueTask(gs_ops[j], localGsKernel[j].kernel, 0, NULL,
                              &syncEvent[i * SUB_PARTITION_NUM + j]);
            }
#if HAVE_APPLY
            setApplyKernel(kernel_apply, getArrangedPartitionID(blkNum / TEST_SCALE - 1), vertexNum);
            clEnqueueTask(apply_ops, kernel_apply, 4,
                          &syncEvent[(blkNum / TEST_SCALE - 1) * SUB_PARTITION_NUM],
                          &applyEvent[(blkNum / TEST_SCALE - 1)]);
#endif
        }

        for (int i = 0; i < SUB_PARTITION_NUM; i++)
        {
            clFinish(gs_ops[i]);
        }
        clFinish(apply_ops);
        fpga_run_end = getCurrentTimestamp();

        /************************************************************************************/

        /* profile */
        DEBUG_PRINTF("profile \n");
        for (int i = 0; i < GLOBAL_BLK_SIZE; i ++)
        {
#if HAVE_APPLY
            unsigned long apply_exec_time = xcl_get_event_duration(applyEvent[i]);
#else
            unsigned long apply_exec_time = 0;
#endif
            for (int j = 0; j < SUB_PARTITION_NUM; j ++)
            {
                unsigned long exec_time = xcl_get_event_duration(*localGsKernel[j].event[i]);
                //clReleaseEvent(localGsKernel[j].event[i]);
                //subPartitions[i * SUB_PARTITION_NUM + j].log.end2end_exe = (fpga_run_end - fpga_run_start) * 1000;
                runtime_total = (fpga_run_end - fpga_run_start) * 1000;
                subPartitions[i * SUB_PARTITION_NUM + j].log.fpga_exe = exec_time;
                subPartitions[i * SUB_PARTITION_NUM + j].log.apply_exe = apply_exec_time;
            }

        }

        /* verification */
        DEBUG_PRINTF("verification \n");
        if (repeat == 0)
        {
            int baseScore = float2int((1.0f - kDamp) / vertexNum);
            for (int i = 0; i < GLOBAL_BLK_SIZE; i ++)
            {
                for (int j = 0; j < SUB_PARTITION_NUM; j ++)
                {
                    partitionGatherScatterCModel(context, device, j, &subPartitions[getArrangedPartitionID(i) * SUB_PARTITION_NUM + j]);
                }
#if HAVE_APPLY
                partitionApplyCModel(context, device, getArrangedPartitionID(i), baseScore);
#endif
            }

        }
        /* log */
        if (repeat > 1)
        {
            double fpga_runtime_total = 0;
            double end2end_runtime_total = 0;
            for (int i = 0;  i < blkNum; i++)
            {
                double max_fpga_exe = 0;
                for (int j = 0; j < SUB_PARTITION_NUM; j ++)
                {
                    if (subPartitions[i * SUB_PARTITION_NUM + j].log.fpga_exe > max_fpga_exe)
                    {
                        max_fpga_exe = subPartitions[i * SUB_PARTITION_NUM + j].log.fpga_exe ;
                    }
                    DEBUG_PRINTF("[INFO]  cu%d exe: %f \n", j, subPartitions[i * SUB_PARTITION_NUM + j].log.fpga_exe / 1000000.0);
                }
                end2end_runtime_total = runtime_total;
                fpga_runtime_total    += max_fpga_exe / 1000000.0;

                DEBUG_PRINTF("[INFO] partedge %f fpga gs: %f ms, apply: %f ms %d, effic %lf  v/e %lf compress %lf \n",
                             subPartitions[i * SUB_PARTITION_NUM].log.end2end_exe,
                             max_fpga_exe / 1000000.0,
                             subPartitions[i * SUB_PARTITION_NUM].log.apply_exe / 1000000.0,
                             (subPartitions[i * SUB_PARTITION_NUM].listEnd - subPartitions[i * SUB_PARTITION_NUM].listStart),
                             (((float)(subPartitions[i * SUB_PARTITION_NUM].listEnd - subPartitions[i * SUB_PARTITION_NUM].listStart)) / subPartitions[i * SUB_PARTITION_NUM].mapedTotalIndex),
                             ((subPartitions[i * SUB_PARTITION_NUM].dstVertexEnd - subPartitions[i * SUB_PARTITION_NUM].dstVertexStart)
                              / ((float)(subPartitions[i * SUB_PARTITION_NUM].listEnd - subPartitions[i * SUB_PARTITION_NUM].listStart))),
                             subPartitions[i * SUB_PARTITION_NUM].compressRatio);

            }
            DEBUG_PRINTF("[INFO] summary e2e %lf fpga %lf\n",
                         end2end_runtime_total,
                         fpga_runtime_total)
        }
        sleep(1);
    }

//clear_host_mem(MEM_ID_TMP_VERTEX_VERIFY);

//transfer_data_from_pl(context, device, MEM_ID_VERTEX_PROP);
//transfer_data_from_pl(context, device, MEM_ID_TMP_VERTEX_PROP);


    return runtime_total;

}

double fpgaProcessing(
    CSR * csr,
    int &blkNum,
    const int &vertexNum,
    const int &edgeNum,
    const int mode // 1 is single thread, 2 is multi-thread
)
{
    double t1 = getCurrentTimestamp();
    partitionFunction(
        csr,
        blkNum,
        context
    );
    double t2 = getCurrentTimestamp();
    double elapsedTime = (t2 - t1) * 1000;
    std::cout << "[INFO] partition takes " << elapsedTime << " ms." << std::endl;
    prepare_fpga_ddr();
    double edge_process_time;

    edge_process_time = launchFPGA();

    return edge_process_time;
}

// CPU thread related to main function
int main(int argc, char **argv) {

    double elapsedTime;
    int startVertexIdx = 1;
    char * xcl_file = NULL;
    if (argc > 1)
    {
        xcl_file = argv[1];
    }
    hardware_init("graph_fpga", xcl_file);
    std::string gName;
    if (argc > 2)
    {
        gName = argv[2];
    }
    else
    {
        gName = "wiki-talk";
    }
    //gName = "twitter-2010";

    std::string mode = "de5_run"; // or harp
    startVertexIdx = getStartIndex();


    printf("start main\n");

    Graph* gptr = createGraph(gName, mode);
    csr = new CSR(*gptr);
    vertexNum = csr->vertexNum;
    edgeNum   = csr->edgeNum;
    free(gptr);

    blkNum = (vertexNum + BLK_SIZE - 1) / BLK_SIZE;
    printf("blkNum %d, ceil num %d \n", blkNum, (int)ceil(static_cast<double>(vertexNum) / BLK_SIZE));

    // init fpga

    mem_init(csr, context);
    kernel_init(program);


    std::cout << "fpga processing." << std::endl;
    processInit(
        vertexNum,
        edgeNum,
        startVertexIdx
    );

    elapsedTime = fpgaProcessing(csr, blkNum, vertexNum, edgeNum, 2);
    std::cout << "[INFO] fpga processing takes " << elapsedTime << " ms." << std::endl;

    freeResources();

    return 0;
}

