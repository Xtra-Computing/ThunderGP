#ifndef __HOST_GRAPH_MISC_INNER_H__
#define __HOST_GRAPH_MISC_INNER_H__

#define DEFAULT_KERNEL_ID       (0)

#define checkStatus(str) {                          \
    if (status != 0 || status != CL_SUCCESS) {      \
        DEBUG_PRINTF("Error code: %d\n", status);   \
        DEBUG_PRINTF("Error: %s\n", str);           \
        acceleratorDeinit();                        \
        exit(-1);                                   \
    }                                               \
}

Graph* createGraph(const std::string &gName, const std::string &mode);

#endif /* __HOST_GRAPH_MISC_H__ */
