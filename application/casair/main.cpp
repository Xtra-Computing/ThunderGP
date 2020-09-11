#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>

#include "host_graph_sw.h"
#include "customize_mem_1.h"

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

    {
        double startStamp, endStamp;
        startStamp = getCurrentTimestamp();
        acceleratorSuperStep(0, &graphDataInfo);
        endStamp = getCurrentTimestamp();
        DEBUG_PRINTF("exe time : %lf \n", endStamp - startStamp);
    }

    write_back_csv<float>("out_s1.csv", MEM_ID_NEWTHETA_S);
    write_back_csv<float>("out_a1.csv", MEM_ID_NEWTHETA_A);
    write_back_csv<float>("out_r1.csv", MEM_ID_NEWTHETA_R);

    acceleratorDeinit();

    return 0;
}
