#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>

#include "host_graph_sw.h"

using namespace std;

graphInfo graphDataInfo;

int main(int argc, char **argv) {

    char * xcl_file = NULL;
    if (argc > 1)
    {
        xcl_file = argv[1];
    }

    std::string gName;
    if (argc > 2)
    {
        gName = argv[2];
    }
    else
    {
        gName = "wiki-talk";
    }
    std::string mode = "normal";
#if 0
    Graph* gptr = createGraph(gName, mode);
    CSR* csr    = new CSR(*gptr);
    csr->save2File(gName);
    free(gptr);
    return 0;
#endif



    DEBUG_PRINTF("start main\n");

    acceleratorInit("graph_fpga", xcl_file);

    acceleratorDataPrepare(gName, mode, &graphDataInfo);

    acceleratorDataPreprocess(&graphDataInfo);

    int runCounter = 0;
    int totalActiveVertices = 1;
    while(totalActiveVertices != 0)
    {
        totalActiveVertices = 0;
        double startStamp, endStamp;
        startStamp = getCurrentTimestamp();

        acceleratorSuperStep(runCounter, &graphDataInfo);

        endStamp = getCurrentTimestamp();

        int *reg = (int *)acceleratorQueryRegister();
        for (int i = 0; i < 32; i++)
        {
            int activeVertices = reg[i];
            totalActiveVertices += activeVertices;
            DEBUG_PRINTF("activeVertice@path_%d : %d \n",i, activeVertices);
        }

        /* profile */
        acceleratorProfile(runCounter, runCounter, &graphDataInfo, endStamp - startStamp);
        runCounter ++;
    }
    acceleratorDeinit();

    return 0;
}

