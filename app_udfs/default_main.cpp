#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>

#include "host_graph_api.h"
#include "host_graph_verification.h"


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
        gName = "rmat-12-4";
    }
    std::string path_graph_dataset = "/home/xinyuc/graph_dataset/";

    DEBUG_PRINTF("start main\n");

    acceleratorInit("graph_fpga", xcl_file);

    acceleratorDataLoad(gName, path_graph_dataset, &graphDataInfo); // it contains heterogeneous memory allocation

    acceleratorDataPreprocess(&graphDataInfo);
    /* for verification */
    acceleratorCModelDataPreprocess(&graphDataInfo);

    for (int runCounter = 0 ; runCounter < 10 ; runCounter ++)
    {
        double startStamp, endStamp;
        startStamp = getCurrentTimestamp();

        acceleratorSuperStep(runCounter, &graphDataInfo);

        endStamp = getCurrentTimestamp();
        /* for verification */
        acceleratorCModelSuperStep(runCounter, &graphDataInfo);
 
        /* for profile */
        acceleratorProfile(runCounter, runCounter, &graphDataInfo, endStamp - startStamp);
    }
    acceleratorDeinit();

    return 0;
}

