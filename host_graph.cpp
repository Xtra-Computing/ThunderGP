
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
    std::string mode = "de5_run"; // or harp




    DEBUG_PRINTF("start main\n");
 
    acceleratorInit("graph_fpga", xcl_file);

    acceleratorDataPrepare(gName,mode,&graphDataInfo);

    acceleratorDataPreprocess(&graphDataInfo);

    for (int superStep = 0 ; superStep < 3 ; superStep ++)
    {
        double startStamp, endStamp;

        startStamp = getCurrentTimestamp();

        acceleratorSuperStep(superStep, &graphDataInfo);

        endStamp = getCurrentTimestamp();

        /* profile */
        accelratorProfile(superStep, &graphDataInfo, endStamp - startStamp);
    }
    acceleratorDeinit();

    return 0;
}

