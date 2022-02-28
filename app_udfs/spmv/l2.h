#ifndef __L2_H__
#define __L2_H__

/* source vertex property process */
inline prop_t preprocessProperty(prop_t srcProp)
{
    return (srcProp);
}

/* source vertex property & edge property */
inline prop_t scatterFunc(prop_t srcProp, prop_t edgeProp)
{
    return ((srcProp) * (edgeProp));
}

/* destination property update dst buffer update */
inline prop_t gatherFunc(prop_t ori, prop_t update)
{
    return ((ori) + (update));
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
