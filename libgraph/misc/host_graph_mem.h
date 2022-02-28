#ifndef __HOST_GRAPH_MEM_H__
#define __HOST_GRAPH_MEM_H__

extern void base_mem_init(cl_context &context);
extern void process_mem_init(cl_context &context);
extern void partition_mem_init(cl_context &context, int blkIndex, int size, int cuIndex);

#endif /* __HOST_GRAPH_MEM_H__ */
