/* DEFAULT_ENTRY in build.mk */

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
    int activeVertices = 1;
    while(activeVertices != 0)
    {

        double startStamp, endStamp;
        startStamp = getCurrentTimestamp();

        acceleratorSuperStep(runCounter, &graphDataInfo);

        endStamp = getCurrentTimestamp();

        int *reg = (int *)acceleratorQueryRegister();
        activeVertices = reg[0];
        DEBUG_PRINTF("activeVertice : %d \n",activeVertices);


        /* profile */
        accelratorProfile(runCounter, runCounter, &graphDataInfo, endStamp - startStamp);
        runCounter++;
    }
    acceleratorDeinit();

    return 0;
}

