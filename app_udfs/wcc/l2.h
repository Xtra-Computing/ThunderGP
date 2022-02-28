#ifndef __L2_H__
#define __L2_H__


/*
    Reference the mapping method in TABLE 1 from hitgraph:
    Zhou, Shijie, et al. "HitGraph: High-throughput graph processing framework on FPGA."
    IEEE Transactions on Parallel and Distributed Systems 30.10 (2019): 2249-2264.

    but we change the min operation to max, for the initialization of BRAM is zero.
    Therefore, do not need another memory interface to reload data from DRAM.
    Note: this implementation ONLY for performance comparison!
*/


/* source vertex property process */
inline prop_t preprocessProperty(prop_t srcProp)
{
    return (srcProp);
}

/* source vertex property & edge property */
inline prop_t scatterFunc(prop_t srcProp, prop_t edgeProp)
{
    return (srcProp);
}

/* destination property update dst buffer update */
inline prop_t gatherFunc(prop_t ori, prop_t update)
{
    return (update > ori) ? update : ori;
}


inline prop_t applyFunc( prop_t tProp,
                                prop_t source,
                                prop_t outDeg,
                                unsigned int (&extra)[APPLY_REF_ARRAY_SIZE],
                                unsigned int arg
                              )
{
    return tProp;
}

#endif /* __L2_H__ */
