#ifndef __HOST_GRAPH_SW_VERIFICATION_H__
#define __HOST_GRAPH_SW_VERIFICATION_H__


int acceleratorProfile (int superStep, int runCounter, graphInfo *info, double exeTime);

int acceleratorCModelDataPreprocess(graphInfo *info);

int acceleratorCModelSuperStep(int superStep, graphInfo *info);


#endif /* __HOST_GRAPH_SW_VERIFICATION_H__ */
