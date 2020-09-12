#ifndef __L2_H__
#define __L2_H__


#define FIXED_SCALE                    (1000000)


/* source vertex property process */
inline prop_t preprocessProperty(prop_t srcProp)
{
    return (srcProp);
}

/* Scatter: return source vertex property "srcProp" as the value of update tuple */
inline prop_t scatterFunc(prop_t srcProp, prop_t edgeProp)
{
    return ((srcProp) * (edgeProp));
}

/* Gather: accumulates the update values from source vertices to original values*/
inline prop_t gatherFunc(prop_t ori, prop_t update)
{
    return ((ori) + (update));
}

#endif /* __L2_H__ */
