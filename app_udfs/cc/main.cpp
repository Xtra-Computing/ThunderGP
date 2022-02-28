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
        gName = "wiki-talk";
    }
    std::string mode = "normal";

    DEBUG_PRINTF("start main\n");

    acceleratorInit("graph_fpga", xcl_file);

    acceleratorDataLoad(gName, mode, &graphDataInfo);

    acceleratorDataPreprocess(&graphDataInfo);
    // 10 times for averaging result;
    for (int j = 0; j < 10 ; j ++)
    {
        int runCounter = 0;
        int totalActiveVertices = 1;
        int closenessCentrality[32];

        //random set 32 nodes to calculated their closeness centrality
        reTransferProp(&graphDataInfo);
        for (int i = 0; i < 32; i++)
        {
            closenessCentrality[i] = 0;
        }
        while (totalActiveVertices != 0)
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
                DEBUG_PRINTF("activeVertice@path_%d : %d \n", i, activeVertices);
            }
            for (int i = 0; i < 32; i++)
            {
                closenessCentrality[i] += reg[i] * runCounter;
            }
            /* profile */
            acceleratorProfile(runCounter, runCounter, &graphDataInfo, endStamp - startStamp);
            runCounter ++;
        }
    }
    //dumpResult(&graphDataInfo);
    acceleratorDeinit();

    return 0;
}

