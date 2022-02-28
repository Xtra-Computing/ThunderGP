#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>

#include "customize_mem_1.h"
#include "host_graph_api.h"
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

    {
        double startStamp, endStamp;
        startStamp = getCurrentTimestamp();
        acceleratorSuperStep(0, &graphDataInfo);
        endStamp = getCurrentTimestamp();
        DEBUG_PRINTF("exe time : %lf \n", endStamp - startStamp);
    }

    write_back_csv<float>("out_s1.csv", MEM_ID_NEWTHETA_S);
    write_back_csv<float>("out_r1.csv", MEM_ID_NEWTHETA_R);

    acceleratorDeinit();

    return 0;
}
