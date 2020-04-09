
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#include <fstream>
#include <vector>

#include "host_graph_sw.h"
#include "unistd.h"



using namespace std;


#define HW_EMU_DEBUG        (0)
#define HW_EMU_DEBUG_SIZE   (16384 * 4)

#define HAVE_APPLY          (1)
#define HAVE_GS             (1)
#define HAVE_FPGA           (1)
#define HAVE_SW             (0)


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

extern gs_cu_t localGsKernel[SUB_PARTITION_NUM];
extern partitionDescriptor partitions[MAX_PARTITIONS_NUM];
extern int partIdTable[MAX_PARTITIONS_NUM];


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

void set_gs_kernel(int partId)
{
#if HAVE_GS
    for (int i = 0; i < SUB_PARTITION_NUM; i++)
    {
        partitionDescriptor *partition = &partitions[partId * SUB_PARTITION_NUM + i];
        int argvi = 0;
        int edgeEnd     = partition->listEnd;
        int sinkStart   = 0;
        int sinkEnd     = VERTEX_MAX;

#if HW_EMU_DEBUG
        edgeEnd         = HW_EMU_DEBUG_SIZE;
#endif
        //DEBUG_PRINTF("gs task in cu [%d] info:\n", i);
        //DEBUG_PRINTF("\tedge  %d %d \n", 0, edgeEnd);
        //DEBUG_PRINTF("\tsink  %d %d \n", sinkStart, sinkEnd);
        clSetKernelArg(localGsKernel[i].kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partition->edgeMap.id));
        clSetKernelArg(localGsKernel[i].kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(localGsKernel[i].prop.id));
        clSetKernelArg(localGsKernel[i].kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partition->edge.id));

        clSetKernelArg(localGsKernel[i].kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partition->tmpProp.id));

        clSetKernelArg(localGsKernel[i].kernel, argvi++, sizeof(int),    &edgeEnd);
        clSetKernelArg(localGsKernel[i].kernel, argvi++, sizeof(int),    &sinkStart);
        clSetKernelArg(localGsKernel[i].kernel, argvi++, sizeof(int),    &sinkEnd);
    }
#endif
}

void set_apply_kernel(int partId)
{
#if HAVE_APPLY
    int argvi = 0;
    int base_score = float2int((1.0f - kDamp) / vertexNum);
    int sink_end   = VERTEX_MAX;
    int offset = 0;//partitions[partId * SUB_PARTITION_NUM].dstVertexStart;

    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(MEM_ID_VERTEX_PROP));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partitions[partId * SUB_PARTITION_NUM + 2].tmpProp.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partitions[partId * SUB_PARTITION_NUM + 1].tmpProp.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partitions[partId * SUB_PARTITION_NUM + 0].tmpProp.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(partitions[partId * SUB_PARTITION_NUM + 3].tmpProp.id));
    /* TODO ping-pong the  propUpdate and prop mem between each iteration */
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(localGsKernel[2].propUpdate.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(localGsKernel[1].propUpdate.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(localGsKernel[0].propUpdate.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(localGsKernel[3].propUpdate.id));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(MEM_ID_OUT_DEG));
    clSetKernelArg(kernel_apply, argvi++, sizeof(cl_mem), get_cl_mem_pointer(MEM_ID_ERROR));

    clSetKernelArg(kernel_apply, argvi++, sizeof(int),    &sink_end);
    clSetKernelArg(kernel_apply, argvi++, sizeof(int),    &offset);
    clSetKernelArg(kernel_apply, argvi++, sizeof(int),    &base_score);
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
    DEBUG_PRINTF("%s", "transfer partitions mem\n");
    for (int i = 0; i < blkNum; i ++) {
        int partition_mem_id[3];
        partition_mem_id[0] = partitions[i].edge.id;
        partition_mem_id[1] = partitions[i].edgeMap.id;
        partition_mem_id[2] = partitions[i].edgeProp.id;
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
#if 1
    int * data = (int *)get_host_mem_pointer(partitions[0].tmpProp.id);
    for (int i = 0; i < VERTEX_MAX; i++)
    {
        data[i] = i * 2;
    }
    int debug_mem_id[2];
    debug_mem_id[0] = partitions[0].tmpProp.id;
    debug_mem_id[1] = partitions[1].tmpProp.id;
    transfer_data_to_pl(context, device, debug_mem_id, ARRAY_SIZE(debug_mem_id));
#endif
}

#define TEST_SCALE          (1)

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
        /* load is same, only one iteration */
        fpga_run_start = getCurrentTimestamp();
        for (int i = 0; i < blkNum / TEST_SCALE; i ++)
        {
            set_gs_kernel(partIdTable[i]);
            if (i > 0)
            {
                set_apply_kernel(partIdTable[i - 1]);
                clEnqueueTask(apply_ops, kernel_apply, 4,
                              &syncEvent[(i - 1) * SUB_PARTITION_NUM],
                              &applyEvent[i - 1]);
            }
            for (int j = 0; j < SUB_PARTITION_NUM; j ++)
            {
                localGsKernel[j].event[i] = &syncEvent[i * SUB_PARTITION_NUM + j];
                clEnqueueTask(gs_ops[j], localGsKernel[j].kernel, 0, NULL,
                              &syncEvent[i * SUB_PARTITION_NUM + j]);
            }
            set_apply_kernel(partIdTable[blkNum / TEST_SCALE - 1]);
            clEnqueueTask(apply_ops, kernel_apply, 4,
                          &syncEvent[(blkNum / TEST_SCALE - 1) * SUB_PARTITION_NUM],
                          &applyEvent[(blkNum / TEST_SCALE - 1)]);
        }

        for (int i = 0; i < SUB_PARTITION_NUM; i++)
        {
            clFinish(gs_ops[i]);
        }
        clFinish(apply_ops);
        fpga_run_end = getCurrentTimestamp();

        /************************************************************************************/

        /* profile */
        for (int i = 0; i < blkNum / TEST_SCALE; i ++)
        {
            unsigned long apply_exec_time = xcl_get_event_duration(applyEvent[i]);
            for (int j = 0; j < SUB_PARTITION_NUM; j ++)
            {
                unsigned long exec_time = xcl_get_event_duration(*localGsKernel[j].event[i]);
                //clReleaseEvent(localGsKernel[j].event[i]);
                //partitions[i * SUB_PARTITION_NUM + j].log.end2end_exe = (fpga_run_end - fpga_run_start) * 1000;
                runtime_total = (fpga_run_end - fpga_run_start) * 1000;
                partitions[i * SUB_PARTITION_NUM + j].log.fpga_exe = exec_time;
                partitions[i * SUB_PARTITION_NUM + j].log.apply_exe = apply_exec_time;
            }

        }

        /* verification */
        if (repeat == 0)
        {
            int baseScore = float2int((1.0f - kDamp) / vertexNum);
            for (int i = 0; i < blkNum / TEST_SCALE; i ++)
            {
                for (int j = 0; j < SUB_PARTITION_NUM; j ++)
                {
                    partitionGatherScatterCModel(context, device, j, &partitions[partIdTable[i] * SUB_PARTITION_NUM + j]);
                }
                partitionApplyCModel(context, device, partIdTable[i], baseScore);
            }

        }
        /* log */
        if (repeat == 2)
        {
            double fpga_runtime_total = 0;
            double end2end_runtime_total = 0;
            for (int i = 0;  i < blkNum; i++)
            {
                double max_fpga_exe = 0;
                for (int j = 0; j < SUB_PARTITION_NUM; j ++)
                {
                    if (partitions[i * SUB_PARTITION_NUM + j].log.fpga_exe > max_fpga_exe)
                    {
                        max_fpga_exe = partitions[i * SUB_PARTITION_NUM + j].log.fpga_exe ;
                    }
                    DEBUG_PRINTF("[INFO]  cu%d exe: %f \n", j, partitions[i * SUB_PARTITION_NUM + j].log.fpga_exe / 1000000.0);
                }
                end2end_runtime_total = runtime_total;
                fpga_runtime_total    += max_fpga_exe / 1000000.0;

                DEBUG_PRINTF("[INFO] partedge %f fpga gs: %f ms, apply: %f ms %d, effic %lf  v/e %lf compress %lf \n",
                             partitions[i * SUB_PARTITION_NUM].log.end2end_exe,
                             max_fpga_exe / 1000000.0,
                             partitions[i * SUB_PARTITION_NUM].log.apply_exe/ 1000000.0,
                             (partitions[i * SUB_PARTITION_NUM].listEnd - partitions[i * SUB_PARTITION_NUM].listStart),
                             (((float)(partitions[i * SUB_PARTITION_NUM].listEnd - partitions[i * SUB_PARTITION_NUM].listStart)) / partitions[i * SUB_PARTITION_NUM].mapedTotalIndex),
                             ((partitions[i * SUB_PARTITION_NUM].dstVertexEnd - partitions[i * SUB_PARTITION_NUM].dstVertexStart)
                              / ((float)(partitions[i * SUB_PARTITION_NUM].listEnd - partitions[i * SUB_PARTITION_NUM].listStart))),
                             partitions[i * SUB_PARTITION_NUM].compressRatio);

            }
            DEBUG_PRINTF("[INFO] summary e2e %lf fpga %lf\n",
                         end2end_runtime_total,
                         fpga_runtime_total)
        }
        sleep(1);
    }

//clear_host_mem(MEM_ID_TMP_VERTEX_VERIFY);

//swVerifyCmodel(task_info);
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
        context,
        partitions
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


    if (gName == "youtube")    startVertexIdx = 320872;
    if (gName == "lj1")        startVertexIdx = 3928512;
    if (gName == "pokec")      startVertexIdx = 182045;
    if (gName == "rmat-19-32") startVertexIdx = 104802;
    if (gName == "rmat-21-32") startVertexIdx = 365723;
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

    //software processing on CPU

#if HAVE_SW
    double begin;
    double end;
    printf ("software PageRank starts.\n");

    processInit(
        vertexNum,
        edgeNum,
        startVertexIdx
    );


    begin = getCurrentTimestamp();

    singleThreadSWProcessing(
        csr,
        blkNum,
        vertexNum,
        startVertexIdx
    );

    end = getCurrentTimestamp();
    elapsedTime = (end - begin) * 1000;

    printf( "[INFO] singleThreadSWProcessing PR takes %lf ms.\n\n", elapsedTime);
#endif

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

