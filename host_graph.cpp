
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

    for (int runCounter = 0 ; runCounter < 3 ; runCounter ++)
    {
        double startStamp, endStamp;

        startStamp = getCurrentTimestamp();

        acceleratorSuperStep(0, &graphDataInfo);

        endStamp = getCurrentTimestamp();

        /* profile */
        accelratorProfile(0, runCounter, &graphDataInfo, endStamp - startStamp);
    }
    acceleratorDeinit();

    return 0;
}

